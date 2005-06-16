/*
 * Copyright notice from original mutt:
 * Copyright (C) 2000-2002 Vsevolod Volkov <vvv@mutt.org.ua>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "mutt.h"
#include "mx.h"
#include "pop.h"
#include "mutt_crypt.h"

#include "lib/mem.h"
#include "lib/str.h"
#include "lib/intl.h"
#include "lib/debug.h"

#include <string.h>
#include <unistd.h>

/* write line to file */
static int fetch_message (char *line, void *file)
{
  FILE *f = (FILE *) file;

  fputs (line, f);
  if (fputc ('\n', f) == EOF)
    return -1;

  return 0;
}

/*
 * Read header
 * returns:
 *  0 on success
 * -1 - conection lost,
 * -2 - invalid command or execution error,
 * -3 - error writing to tempfile
 */
static pop_query_status pop_read_header (POP_DATA * pop_data, HEADER * h)
{
  FILE *f;
  int index;
  pop_query_status ret;
  long length;
  char buf[LONG_STRING];
  char tempfile[_POSIX_PATH_MAX];

  mutt_mktemp (tempfile);
  if (!(f = safe_fopen (tempfile, "w+"))) {
    mutt_perror (tempfile);
    return PFD_FUNCT_ERROR;
  }

  snprintf (buf, sizeof (buf), "LIST %d\r\n", h->refno);
  ret = pop_query (pop_data, buf, sizeof (buf));
  if (ret == PQ_OK) {
    sscanf (buf, "+OK %d %ld", &index, &length);

    snprintf (buf, sizeof (buf), "TOP %d 0\r\n", h->refno);
    ret = pop_fetch_data (pop_data, buf, NULL, fetch_message, f);

    if (pop_data->cmd_top == CMD_UNKNOWN) {
      if (ret == PQ_OK) {
        pop_data->cmd_top = CMD_AVAILABLE;

        debug_print (1, ("set TOP capability\n"));
      }

      if (ret == PQ_ERR) {
        pop_data->cmd_top = CMD_NOT_AVAILABLE;

        debug_print (1, ("unset TOP capability\n"));
        snprintf (pop_data->err_msg, sizeof (pop_data->err_msg),
                  _("Command TOP is not supported by server."));
      }
    }
  }

  switch (ret) {
  case PQ_OK:
    {
      rewind (f);
      h->env = mutt_read_rfc822_header (f, h, 0, 0);
      h->content->length = length - h->content->offset + 1;
      rewind (f);
      while (!feof (f)) {
        h->content->length--;
        fgets (buf, sizeof (buf), f);
      }
      break;
    }
  case PQ_ERR:
    {
      mutt_error ("%s", pop_data->err_msg);
      break;
    }
  case PFD_FUNCT_ERROR:
    {
      mutt_error _("Can't write header to temporary file!");

      break;
    }
  }

  fclose (f);
  unlink (tempfile);
  return ret;
}

/* parse UIDL */
static int fetch_uidl (char *line, void *data)
{
  int i, index;
  CONTEXT *ctx = (CONTEXT *) data;
  POP_DATA *pop_data = (POP_DATA *) ctx->data;

  sscanf (line, "%d %s", &index, line);
  for (i = 0; i < ctx->msgcount; i++)
    if (!safe_strcmp (line, ctx->hdrs[i]->data))
      break;

  if (i == ctx->msgcount) {
    debug_print (1, ("new header %d %s\n", index, line));

    if (i >= ctx->hdrmax)
      mx_alloc_memory (ctx);

    ctx->msgcount++;
    ctx->hdrs[i] = mutt_new_header ();
    ctx->hdrs[i]->data = safe_strdup (line);
  }
  else if (ctx->hdrs[i]->index != index - 1)
    pop_data->clear_cache = 1;

  ctx->hdrs[i]->refno = index;
  ctx->hdrs[i]->index = index - 1;

  return 0;
}

/*
 * Read headers
 * returns:
 *  0 on success
 * -1 - conection lost,
 * -2 - invalid command or execution error,
 * -3 - error writing to tempfile
 */
static int pop_fetch_headers (CONTEXT * ctx)
{
  int i, old_count, new_count;
  pop_query_status ret;
  POP_DATA *pop_data = (POP_DATA *) ctx->data;

  time (&pop_data->check_time);
  pop_data->clear_cache = 0;

  for (i = 0; i < ctx->msgcount; i++)
    ctx->hdrs[i]->refno = -1;

  old_count = ctx->msgcount;
  ret = pop_fetch_data (pop_data, "UIDL\r\n", NULL, fetch_uidl, ctx);
  new_count = ctx->msgcount;
  ctx->msgcount = old_count;

  if (pop_data->cmd_uidl == CMD_UNKNOWN) {
    if (ret == PQ_OK) {
      pop_data->cmd_uidl = CMD_AVAILABLE;

      debug_print (1, ("set UIDL capability\n"));
    }

    if (ret == PQ_ERR && pop_data->cmd_uidl == CMD_UNKNOWN) {
      pop_data->cmd_uidl = CMD_NOT_AVAILABLE;

      debug_print (1, ("unset UIDL capability\n"));
      snprintf (pop_data->err_msg, sizeof (pop_data->err_msg),
                _("Command UIDL is not supported by server."));
    }
  }

  if (ret == PQ_OK) {
    for (i = 0; i < old_count; i++)
      if (ctx->hdrs[i]->refno == -1)
        ctx->hdrs[i]->deleted = 1;

    for (i = old_count; i < new_count; i++) {
      mutt_message (_("Fetching message headers... [%d/%d]"),
                    i + 1 - old_count, new_count - old_count);

      ret = pop_read_header (pop_data, ctx->hdrs[i]);
      if (ret != PQ_OK)
        break;

      ctx->msgcount++;
    }

    if (i > old_count)
      mx_update_context (ctx, i - old_count);
  }

  if (ret != PQ_OK) {
    for (i = ctx->msgcount; i < new_count; i++)
      mutt_free_header (&ctx->hdrs[i]);
    return ret;
  }

  mutt_clear_error ();
  return (new_count - old_count);
}

/* open POP mailbox - fetch only headers */
int pop_open_mailbox (CONTEXT * ctx)
{
  int ret;
  char buf[LONG_STRING];
  CONNECTION *conn;
  ACCOUNT acct;
  POP_DATA *pop_data;
  ciss_url_t url;

  if (pop_parse_path (ctx->path, &acct)) {
    mutt_error (_("%s is an invalid POP path"), ctx->path);
    mutt_sleep (2);
    return -1;
  }

  mutt_account_tourl (&acct, &url);
  url.path = NULL;
  url_ciss_tostring (&url, buf, sizeof (buf), 0);
  conn = mutt_conn_find (NULL, &acct);
  if (!conn)
    return -1;

  FREE (&ctx->path);
  ctx->path = safe_strdup (buf);

  pop_data = safe_calloc (1, sizeof (POP_DATA));
  pop_data->conn = conn;
  ctx->data = pop_data;

  if (pop_open_connection (pop_data) < 0)
    return -1;

  conn->data = pop_data;

  FOREVER {
    if (pop_reconnect (ctx) != PQ_OK)
      return -1;

    ctx->size = pop_data->size;

    mutt_message _("Fetching list of messages...");

    ret = pop_fetch_headers (ctx);

    if (ret >= 0)
      return 0;

    if (ret < -1) {
      mutt_sleep (2);
      return -1;
    }
  }
}

/* delete all cached messages */
static void pop_clear_cache (POP_DATA * pop_data)
{
  int i;

  if (!pop_data->clear_cache)
    return;

  debug_print (1, ("delete cached messages\n"));

  for (i = 0; i < POP_CACHE_LEN; i++) {
    if (pop_data->cache[i].path) {
      unlink (pop_data->cache[i].path);
      FREE (&pop_data->cache[i].path);
    }
  }
}

/* close POP mailbox */
void pop_close_mailbox (CONTEXT * ctx)
{
  POP_DATA *pop_data = (POP_DATA *) ctx->data;

  if (!pop_data)
    return;

  pop_logout (ctx);

  if (pop_data->status != POP_NONE)
    mutt_socket_close (pop_data->conn);

  pop_data->status = POP_NONE;

  pop_data->clear_cache = 1;
  pop_clear_cache (pop_data);

  if (!pop_data->conn->data)
    mutt_socket_free (pop_data->conn);

  return;
}

/* fetch message from POP server */
int pop_fetch_message (MESSAGE * msg, CONTEXT * ctx, int msgno)
{
  int ret;
  void *uidl;
  char buf[LONG_STRING];
  char path[_POSIX_PATH_MAX];
  char *m = _("Fetching message...");
  POP_DATA *pop_data = (POP_DATA *) ctx->data;
  POP_CACHE *cache;
  HEADER *h = ctx->hdrs[msgno];

  /* see if we already have the message in our cache */
  cache = &pop_data->cache[h->index % POP_CACHE_LEN];

  if (cache->path) {
    if (cache->index == h->index) {
      /* yes, so just return a pointer to the message */
      msg->fp = fopen (cache->path, "r");
      if (msg->fp)
        return 0;

      mutt_perror (cache->path);
      mutt_sleep (2);
      return -1;
    }
    else {
      /* clear the previous entry */
      unlink (cache->path);
      FREE (&cache->path);
    }
  }

  FOREVER {
    if (pop_reconnect (ctx) != PQ_OK)
      return -1;

    /* verify that massage index is correct */
    if (h->refno < 0) {
      mutt_error
        _("The message index is incorrect. Try reopening the mailbox.");
      mutt_sleep (2);
      return -1;
    }

    mutt_message (m);

    mutt_mktemp (path);
    msg->fp = safe_fopen (path, "w+");
    if (!msg->fp) {
      mutt_perror (path);
      mutt_sleep (2);
      return -1;
    }

    snprintf (buf, sizeof (buf), "RETR %d\r\n", h->refno);

    ret = pop_fetch_data (pop_data, buf, m, fetch_message, msg->fp);
    if (ret == PQ_OK)
      break;

    safe_fclose (&msg->fp);
    unlink (path);

    if (ret == PQ_ERR) {
      mutt_error ("%s", pop_data->err_msg);
      mutt_sleep (2);
      return -1;
    }

    if (ret == PFD_FUNCT_ERROR) {
      mutt_error _("Can't write message to temporary file!");

      mutt_sleep (2);
      return -1;
    }
  }

  /* Update the header information.  Previously, we only downloaded a
   * portion of the headers, those required for the main display.
   */
  cache->index = h->index;
  cache->path = safe_strdup (path);
  rewind (msg->fp);
  uidl = h->data;
  mutt_free_envelope (&h->env);
  h->env = mutt_read_rfc822_header (msg->fp, h, 0, 0);
  h->data = uidl;
  h->lines = 0;
  fgets (buf, sizeof (buf), msg->fp);
  while (!feof (msg->fp)) {
    ctx->hdrs[msgno]->lines++;
    fgets (buf, sizeof (buf), msg->fp);
  }

  h->content->length = ftell (msg->fp) - h->content->offset;

  /* This needs to be done in case this is a multipart message */
  if (!WithCrypto)
    h->security = crypt_query (h->content);

  mutt_clear_error ();
  rewind (msg->fp);

  return 0;
}

/* update POP mailbox - delete messages from server */
pop_query_status pop_sync_mailbox (CONTEXT * ctx, int unused, int *index_hint)
{
  int i;
  pop_query_status ret;
  char buf[LONG_STRING];
  POP_DATA *pop_data = (POP_DATA *) ctx->data;

  pop_data->check_time = 0;

  FOREVER {
    if (pop_reconnect (ctx) != PQ_OK)
      return PQ_NOT_CONNECTED;

    mutt_message (_("Marking %d messages deleted..."), ctx->deleted);

    for (i = 0, ret = 0; ret == 0 && i < ctx->msgcount; i++) {
      if (ctx->hdrs[i]->deleted) {
        snprintf (buf, sizeof (buf), "DELE %d\r\n", ctx->hdrs[i]->refno);
        ret = pop_query (pop_data, buf, sizeof (buf));
      }
    }

    if (ret == PQ_OK) {
      strfcpy (buf, "QUIT\r\n", sizeof (buf));
      ret = pop_query (pop_data, buf, sizeof (buf));
    }

    if (ret == PQ_OK) {
      pop_data->clear_cache = 1;
      pop_clear_cache (pop_data);
      pop_data->status = POP_DISCONNECTED;
      return PQ_OK;
    }

    if (ret == PQ_ERR) {
      mutt_error ("%s", pop_data->err_msg);
      mutt_sleep (2);
      return PQ_NOT_CONNECTED;
    }
  }
}

/* Check for new messages and fetch headers */
int pop_check_mailbox (CONTEXT * ctx, int *index_hint, int unused)
{
  int ret;
  POP_DATA *pop_data = (POP_DATA *) ctx->data;

  if ((pop_data->check_time + PopCheckTimeout) > time (NULL))
    return 0;

  pop_logout (ctx);

  mutt_socket_close (pop_data->conn);

  if (pop_open_connection (pop_data) < 0)
    return -1;

  ctx->size = pop_data->size;

  mutt_message _("Checking for new messages...");

  ret = pop_fetch_headers (ctx);
  pop_clear_cache (pop_data);

  if (ret < 0)
    return -1;

  if (ret > 0)
    return M_NEW_MAIL;

  return 0;
}

/* Fetch messages and save them in $spoolfile */
void pop_fetch_mail (void)
{
  char buffer[LONG_STRING];
  char msgbuf[SHORT_STRING];
  char *url, *p;
  int i, delanswer, last = 0, msgs, bytes, rset = 0;
  pop_query_status ret;
  CONNECTION *conn;
  CONTEXT ctx;
  MESSAGE *msg = NULL;
  ACCOUNT acct;
  POP_DATA *pop_data;

  if (!PopHost) {
    mutt_error _("POP host is not defined.");

    return;
  }

  url = p = safe_calloc (strlen (PopHost) + 7, sizeof (char));
  if (url_check_scheme (PopHost) == U_UNKNOWN) {
    strcpy (url, "pop://");     /* __STRCPY_CHECKED__ */
    p = strchr (url, '\0');
  }
  strcpy (p, PopHost);          /* __STRCPY_CHECKED__ */

  ret = pop_parse_path (url, &acct);
  FREE (&url);
  if (ret) {
    mutt_error (_("%s is an invalid POP path"), PopHost);
    return;
  }

  conn = mutt_conn_find (NULL, &acct);
  if (!conn)
    return;

  pop_data = safe_calloc (1, sizeof (POP_DATA));
  pop_data->conn = conn;

  if (pop_open_connection (pop_data) < 0) {
    mutt_socket_free (pop_data->conn);
    FREE (&pop_data);
    return;
  }

  conn->data = pop_data;

  mutt_message _("Checking for new messages...");

  /* find out how many messages are in the mailbox. */
  strfcpy (buffer, "STAT\r\n", sizeof (buffer));
  ret = pop_query (pop_data, buffer, sizeof (buffer));
  if (ret == PQ_NOT_CONNECTED)
    goto fail;
  if (ret == PQ_ERR) {
    mutt_error ("%s", pop_data->err_msg);
    goto finish;
  }

  sscanf (buffer, "+OK %d %d", &msgs, &bytes);

  /* only get unread messages */
  if (msgs > 0 && option (OPTPOPLAST)) {
    strfcpy (buffer, "LAST\r\n", sizeof (buffer));
    ret = pop_query (pop_data, buffer, sizeof (buffer));
    if (ret == PQ_NOT_CONNECTED)
      goto fail;
    if (ret == PQ_OK)
      sscanf (buffer, "+OK %d", &last);
  }

  if (msgs <= last) {
    mutt_message _("No new mail in POP mailbox.");

    goto finish;
  }

  if (mx_open_mailbox (NONULL (Spoolfile), M_APPEND, &ctx) == NULL)
    goto finish;

  delanswer =
    query_quadoption (OPT_POPDELETE, _("Delete messages from server?"));

  snprintf (msgbuf, sizeof (msgbuf), _("Reading new messages (%d bytes)..."),
            bytes);
  mutt_message ("%s", msgbuf);

  for (i = last + 1; i <= msgs; i++) {
    if ((msg = mx_open_new_message (&ctx, NULL, M_ADD_FROM)) == NULL)
      ret = -3;
    else {
      snprintf (buffer, sizeof (buffer), "RETR %d\r\n", i);
      ret = pop_fetch_data (pop_data, buffer, NULL, fetch_message, msg->fp);
      if (ret == PFD_FUNCT_ERROR)
        rset = 1;

      if (ret == PQ_OK && mx_commit_message (msg, &ctx) != 0) {
        rset = 1;
        ret = PFD_FUNCT_ERROR;
      }

      mx_close_message (&msg);
    }

    if (ret == PQ_OK && delanswer == M_YES) {
      /* delete the message on the server */
      snprintf (buffer, sizeof (buffer), "DELE %d\r\n", i);
      ret = pop_query (pop_data, buffer, sizeof (buffer));
    }

    if (ret == PQ_NOT_CONNECTED) {
      mx_close_mailbox (&ctx, NULL);
      goto fail;
    }
    if (ret == PQ_ERR) {
      mutt_error ("%s", pop_data->err_msg);
      break;
    }
    if (ret == -3) { /* this is -3 when ret != 0, because it will keep the value from before *gna* */
      mutt_error _("Error while writing mailbox!");

      break;
    }

    mutt_message (_("%s [%d of %d messages read]"), msgbuf, i - last,
                  msgs - last);
  }

  mx_close_mailbox (&ctx, NULL);

  if (rset) {
    /* make sure no messages get deleted */
    strfcpy (buffer, "RSET\r\n", sizeof (buffer));
    if (pop_query (pop_data, buffer, sizeof (buffer)) == PQ_NOT_CONNECTED)
      goto fail;
  }

finish:
  /* exit gracefully */
  strfcpy (buffer, "QUIT\r\n", sizeof (buffer));
  if (pop_query (pop_data, buffer, sizeof (buffer)) == PQ_NOT_CONNECTED)
    goto fail;
  mutt_socket_close (conn);
  FREE (&pop_data);
  return;

fail:
  mutt_error _("Server closed connection!");
  mutt_socket_close (conn);
  FREE (&pop_data);
}
