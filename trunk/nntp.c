/*
 * Copyright (C) 1998 Brandon Long <blong@fiction.net>
 * Copyright (C) 1999 Andrej Gritsenko <andrej@lucky.net>
 * Copyright (C) 2000-2002 Vsevolod Volkov <vvv@mutt.org.ua>
 * 
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 * 
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 * 
 *     You should have received a copy of the GNU General Public License
 *     along with this program; if not, write to the Free Software
 *     Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */ 

#include "mutt.h"
#include "mutt_curses.h"
#include "sort.h"
#include "mx.h"
#include "mime.h"
#include "rfc1524.h"
#include "rfc2047.h"
#include "mailbox.h"
#include "nntp.h"

#ifdef HAVE_PGP
#include "pgp.h"
#endif

#ifdef HAVE_SMIME
#include "smime.h"
#endif

#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

static unsigned int _checked = 0;

#ifdef DEBUG
static void nntp_error (const char *where, const char *msg)
{
  dprint (1, (debugfile, "nntp_error(): unexpected response in %s: %s\n", where, msg));
}
#endif /* DEBUG */

static int nntp_auth (NNTP_SERVER *serv)
{
  CONNECTION *conn = serv->conn;
  char buf[STRING];
  unsigned char flags = conn->account.flags;

  if (mutt_account_getuser (&conn->account) || !conn->account.user[0] ||
      mutt_account_getpass (&conn->account) || !conn->account.pass[0])
  {
    conn->account.flags = flags;
    return -2;
  }

  mutt_message _("Logging in...");

  snprintf (buf, sizeof (buf), "AUTHINFO USER %s\r\n", conn->account.user);
  mutt_socket_write (conn, buf);
  if (mutt_socket_readln (buf, sizeof (buf), conn) < 0)
  {
    conn->account.flags = flags;
    return -1;
  }

#ifdef DEBUG
  /* don't print the password unless we're at the ungodly debugging level */
  if (debuglevel < M_SOCK_LOG_FULL)
    dprint (M_SOCK_LOG_CMD, (debugfile, "> AUTHINFO PASS *\n"));
#endif
  snprintf (buf, sizeof (buf), "AUTHINFO PASS %s\r\n", conn->account.pass);
  mutt_socket_write_d (conn, buf, M_SOCK_LOG_FULL);
  if (mutt_socket_readln (buf, sizeof (buf), conn) < 0)
  {
    conn->account.flags = flags;
    return -1;
  }

  if (mutt_strncmp ("281", buf, 3))
  {
    conn->account.flags = flags;
    mutt_error _("Login failed.");
    sleep (2);
    return -3;
  }

  return 0;
}

static int nntp_connect_error (NNTP_SERVER *serv)
{
  serv->status = NNTP_NONE;
  mutt_socket_close (serv->conn);
  mutt_error _("Server closed connection!");
  sleep (2);
  return -1;
}

static int nntp_connect_and_auth (NNTP_SERVER *serv)
{
  CONNECTION *conn = serv->conn;
  char buf[STRING];
  int rc;

  serv->status = NNTP_NONE;

  if (mutt_socket_open (conn) < 0)
    return -1;

  if (mutt_socket_readln (buf, sizeof (buf), conn) < 0)
    return nntp_connect_error (serv);

  if (!mutt_strncmp ("200", buf, 3))
    mutt_message (_("Connected to %s. Posting ok."), conn->account.host);
  else if (!mutt_strncmp ("201", buf, 3))
    mutt_message (_("Connected to %s. Posting NOT ok."), conn->account.host);
  else
  {
    mutt_socket_close (conn);
    mutt_remove_trailing_ws (buf);
    mutt_error ("%s", buf);
    sleep (2);
    return -1;
  }

  sleep (1);

  /* Tell INN to switch to mode reader if it isn't so. Ignore all
   returned codes and messages. */
  mutt_socket_write (conn, "MODE READER\r\n");
  if (mutt_socket_readln (buf, sizeof (buf), conn) < 0)
    return nntp_connect_error (serv);

  mutt_socket_write (conn, "STAT\r\n");
  if (mutt_socket_readln (buf, sizeof (buf), conn) < 0)
    return nntp_connect_error (serv);

  if (!(conn->account.flags & M_ACCT_USER) && mutt_strncmp ("480", buf, 3))
  {
    serv->status = NNTP_OK;
    return 0;
  }

  rc = nntp_auth (serv);
  if (rc == -1)
    return nntp_connect_error (serv);
  if (rc == -2)
  {
    mutt_socket_close (conn);
    serv->status = NNTP_BYE;
    return -1;
  }
  if (rc < 0)
  {
    mutt_socket_close (conn);
    mutt_error _("Login failed.");
    sleep (2);
    return -1;
  }
  serv->status = NNTP_OK;
  return 0;
}

static int nntp_attempt_features (NNTP_SERVER *serv)
{
  char buf[LONG_STRING];
  CONNECTION *conn = serv->conn;

  mutt_socket_write (conn, "XOVER\r\n");
  if (mutt_socket_readln (buf, sizeof (buf), conn) < 0)
    return nntp_connect_error (serv);
  if (mutt_strncmp ("500", buf, 3))
    serv->hasXOVER = 1;

  mutt_socket_write (conn, "XPAT\r\n");
  if (mutt_socket_readln (buf, sizeof (buf), conn) < 0)
    return nntp_connect_error (serv);
  if (mutt_strncmp ("500", buf, 3))
    serv->hasXPAT = 1;

  mutt_socket_write (conn, "XGTITLE +\r\n");
  if (mutt_socket_readln (buf, sizeof (buf), conn) < 0)
    return nntp_connect_error (serv);
  if (mutt_strncmp ("500", buf, 3))
    serv->hasXGTITLE = 1;

  if (!mutt_strncmp ("282", buf, 3))
  {
    do
    {
      if (mutt_socket_readln (buf, sizeof (buf), conn) < 0)
	return nntp_connect_error (serv);
    } while (!(buf[0] == '.' && buf[1] == '\0'));
  }

  return 0;
}

static int nntp_open_connection (NNTP_SERVER *serv)
{
  if (serv->status == NNTP_OK)
    return 0;
  if (serv->status == NNTP_BYE)
    return -1;
  if (nntp_connect_and_auth (serv) < 0)
    return -1;
  if (nntp_attempt_features (serv) < 0)
    return -1;
  return 0;
}

static int nntp_reconnect (NNTP_SERVER *serv)
{
  char buf[SHORT_STRING];

  mutt_socket_close (serv->conn);

  FOREVER
  {
    if (nntp_connect_and_auth (serv) == 0)
      return 0;

    snprintf (buf, sizeof (buf), _("Connection to %s lost. Reconnect?"),
				serv->conn->account.host);
    if (query_quadoption (OPT_NNTPRECONNECT, buf) != M_YES)
    {
      serv->status = NNTP_BYE;
      return -1;
    }
  }
}

/* Send data from line[LONG_STRING] and receive answer to same line */
static int mutt_nntp_query (NNTP_DATA *data, char *line, size_t linelen)
{
  char buf[LONG_STRING];
  int done = TRUE;

  if (data->nserv->status == NNTP_BYE)
    return -1;

  do
  {
    if (*line)
    {
      mutt_socket_write (data->nserv->conn, line);
    }
    else if (data->group)
    {
      snprintf (buf, sizeof (buf), "GROUP %s\r\n", data->group);
      mutt_socket_write (data->nserv->conn, buf);
    }

    done = TRUE;
    if (mutt_socket_readln (buf, sizeof (buf), data->nserv->conn) < 0)
    {
      if (nntp_reconnect (data->nserv) < 0)
 	return -1;

      if (data->group)
      {
	snprintf (buf, sizeof (buf), "GROUP %s\r\n", data->group);
	mutt_socket_write (data->nserv->conn, buf);
	if (mutt_socket_readln (buf, sizeof (buf), data->nserv->conn) < 0)
	  return -1;
      }
      if (*line)
	done = FALSE;
    }
    else if ((!mutt_strncmp ("480", buf, 3)) && nntp_auth (data->nserv) < 0)
      return -1;
  } while (!done);

  strfcpy (line, buf, linelen);
  return 0;
}

/*
 * This function calls  funct(*line, *data)  for each received line,
 * funct(NULL, *data)  if  rewind(*data)  needs, exits when fail or done.
 * Returned codes:
 *  0 - successful,
 *  1 - correct but not performed (may be, have to be continued),
 * -1 - conection lost,
 * -2 - invalid command or execution error,
 * -3 - error in funct(*line, *data).
 */
static int mutt_nntp_fetch (NNTP_DATA *nntp_data, char *query, char *msg,
		int (*funct) (char *, void *), void *data, int tagged)
{
  char buf[LONG_STRING];
  char *inbuf, *p;
  int done = FALSE;
  int chunk, line;
  size_t lenbuf = 0;
  int ret;

  do
  {
    strfcpy (buf, query, sizeof (buf));
    if (mutt_nntp_query (nntp_data, buf, sizeof (buf)) < 0)
      return -1;
    if (buf[0] == '5')
      return -2;
    if (buf[0] != '2')
      return 1;

    ret = 0;
    line = 0;
    inbuf = safe_malloc (sizeof (buf));

    FOREVER
    {
      chunk = mutt_socket_readln_d (buf, sizeof (buf), nntp_data->nserv->conn,
				    M_SOCK_LOG_HDR);
      if (chunk < 0)
	break;

      p = buf;
      if (!lenbuf && buf[0] == '.')
      {
	if (buf[1] == '\0')
	{
	  done = TRUE;
	  break;
	}
	if (buf[1] == '.')
	  p++;
      }

      strfcpy (inbuf + lenbuf, p, sizeof (buf));

      if (chunk >= sizeof (buf))
      {
	lenbuf += strlen (p);
      }
      else
      {
	line++;
	if (msg && ReadInc && (line % ReadInc == 0)) {
	  if (tagged)
	    mutt_message (_("%s (tagged: %d) %d"), msg, tagged, line);
	  else
	    mutt_message ("%s %d", msg, line);
	}

	if (ret == 0 && funct (inbuf, data) < 0)
	  ret = -3;
	lenbuf = 0;
      }

      safe_realloc (&inbuf, lenbuf + sizeof (buf));
    }
    FREE (&inbuf);
    funct (NULL, data);
  }
  while (!done);
  return ret;
}

static int nntp_read_tempfile (char *line, void *file)
{
  FILE *f = (FILE *)file;

  if (!line)
    rewind (f);
  else
  {
    fputs (line, f);
    if (fputc ('\n', f) == EOF)
      return -1;
  }
  return 0;
}

static void nntp_parse_xref (CONTEXT *ctx, char *group, char *xref, HEADER *h)
{
  register char *p, *b;
  register char *colon = NULL;

  b = p = xref;
  while (*p)
  {
    /* skip to next word */
    b = p;
    while (*b && ((*b == ' ') || (*b == '\t'))) b++;
    p = b;
    colon = NULL;
    /* skip to end of word */
    while (*p && (*p != ' ') && (*p != '\t')) 
    {
      if (*p == ':')
	colon = p;
      p++;
    }
    if (*p)
    {
      *p = '\0';
      p++;
    }
    if (colon)
    {
      *colon = '\0';
      colon++;
      nntp_get_status (ctx, h, p, atoi(colon));
      if (h && h->article_num == 0 && mutt_strcmp (group, b) == 0)
	h->article_num = atoi(colon);
    }
  }
}

/*
 * returns:
 *  0 on success
 *  1 if article not found
 * -1 if read or write error on tempfile or socket
 */
static int nntp_read_header (CONTEXT *ctx, const char *msgid, int article_num)
{
  NNTP_DATA *nntp_data = ((NNTP_DATA *)ctx->data);
  FILE *f;
  char buf[LONG_STRING];
  char tempfile[_POSIX_PATH_MAX];
  int ret;
  HEADER *h = ctx->hdrs[ctx->msgcount];

  mutt_mktemp (tempfile);
  if (!(f = safe_fopen (tempfile, "w+")))
    return -1;

  if (!msgid)
    snprintf (buf, sizeof (buf), "HEAD %d\r\n", article_num);
  else
    snprintf (buf, sizeof (buf), "HEAD %s\r\n", msgid);

  ret = mutt_nntp_fetch (nntp_data, buf, NULL, nntp_read_tempfile, f, 0);
  if (ret)
  {
#ifdef DEBUG
    if (ret != -1)
      dprint(1, (debugfile, "nntp_read_header: %s\n", buf));
#endif
    fclose (f);
    unlink (tempfile);
    return (ret == -1 ? -1 : 1);
  }

  h->article_num = article_num;
  h->env = mutt_read_rfc822_header (f, h, 0, 0);
  fclose (f);
  unlink (tempfile);

  if (h->env->xref != NULL)
    nntp_parse_xref (ctx, nntp_data->group, h->env->xref, h);
  else if (h->article_num == 0 && msgid)
  {
    snprintf (buf, sizeof (buf), "STAT %s\r\n", msgid);
    if (mutt_nntp_query (nntp_data, buf, sizeof (buf)) == 0)
      h->article_num = atoi (buf + 4);
  }

  return 0;
}

static int parse_description (char *line, void *n)
{
#define news ((NNTP_SERVER *) n)
  register char *d = line;
  NNTP_DATA *data;

  if (!line)
    return 0;
  while (*d && *d != '\t' && *d != ' ') d++;
    *d = 0;
  d++;
  while (*d && (*d == '\t' || *d == ' ')) d++;
  dprint (2, (debugfile, "group: %s, desc: %s\n", line, d));
  if ((data = (NNTP_DATA *) hash_find (news->newsgroups, line)) != NULL &&
	mutt_strcmp (d, data->desc))
  {
    FREE (&data->desc);
    data->desc = safe_strdup (d);
  }
  return 0;
#undef news
}

static void nntp_get_desc (NNTP_DATA *data, char *mask, char *msg)
{
  char buf[STRING];

  if (!option (OPTLOADDESC) || !data || !data->nserv)
    return;

  /* Get newsgroup description, if we can */
  if (data->nserv->hasXGTITLE)
    snprintf (buf, sizeof (buf), "XGTITLE %s\r\n", mask);
  else
    snprintf (buf, sizeof (buf), "LIST NEWSGROUPS %s\r\n", mask);
  if (mutt_nntp_fetch (data, buf, msg, parse_description, data->nserv, 0) != 0)
  {
#ifdef DEBUG
    nntp_error ("nntp_get_desc()", buf);
#endif
  }
}

/*
 * XOVER returns a tab separated list of:
 * id|subject|from|date|Msgid|references|bytes|lines|xref
 *
 * This has to duplicate some of the functionality of 
 * mutt_read_rfc822_header(), since it replaces the call to that (albeit with
 * a limited number of headers which are "parsed" by placement in the list)
 */
static int nntp_parse_xover (CONTEXT *ctx, char *buf, HEADER *hdr)
{
  NNTP_DATA *nntp_data = (NNTP_DATA *) ctx->data;
  char *p, *b;
  int x, done = 0;

  hdr->env = mutt_new_envelope();
  hdr->env->newsgroups = safe_strdup (nntp_data->group);
  hdr->content = mutt_new_body();
  hdr->content->type = TYPETEXT;
  hdr->content->subtype = safe_strdup ("plain");
  hdr->content->encoding = ENC7BIT;
  hdr->content->disposition = DISPINLINE;
  hdr->content->length = -1;
  b = p = buf;

  for (x = 0; !done && x < 9; x++)
  {
    /* if from file, need to skip newline character */
    while (*p && *p != '\n' && *p != '\t') p++;
    if (!*p) done++;
    *p = '\0';
    p++;
    switch (x)
    {
      case 0:

	hdr->article_num = atoi (b);
	nntp_get_status (ctx, hdr, NULL, hdr->article_num);
	break;
      case 1:
	hdr->env->subject = safe_strdup (b);
	/* Now we need to do the things which would normally be done in 
	 * mutt_read_rfc822_header() */
	if (hdr->env->subject)
	{
	  regmatch_t pmatch[1];

	  rfc2047_decode (&hdr->env->subject);

	  if (regexec (ReplyRegexp.rx, hdr->env->subject, 1, pmatch, 0) == 0)
	    hdr->env->real_subj = hdr->env->subject + pmatch[0].rm_eo;
	  else
	    hdr->env->real_subj = hdr->env->subject;
	}
	break;
      case 2:
	rfc822_free_address (&hdr->env->from);
	hdr->env->from = rfc822_parse_adrlist (hdr->env->from, b);
	rfc2047_decode_adrlist (hdr->env->from);
	break;
      case 3:
	hdr->date_sent = mutt_parse_date (b, hdr);
	hdr->received = hdr->date_sent;
	break;
      case 4:
	FREE (&hdr->env->message_id);
	hdr->env->message_id = safe_strdup (b);
	break;
      case 5:
	mutt_free_list (&hdr->env->references);
	hdr->env->references = mutt_parse_references (b, 0);
	break;
      case 6:
	hdr->content->length = atoi (b);
	break;
      case 7:
	hdr->lines = atoi (b);
	break;
      case 8:
	if (!hdr->read)
	  FREE (&hdr->env->xref);
	  b = b + 6;	/* skips the "Xref: " */
	  hdr->env->xref = safe_strdup (b);
	  nntp_parse_xref (ctx, nntp_data->group, b, hdr);
    }
    if (!*p)
      return -1;
    b = p;
  }
  return 0;
}

typedef struct
{
  CONTEXT *ctx;
  unsigned int base;
  unsigned int first;
  unsigned int last;
  unsigned short *messages;
  char* msg;
} FETCH_CONTEXT;

#define fc ((FETCH_CONTEXT *) c)
static int nntp_fetch_numbers (char *line, void *c)
{
  unsigned int num;

  if (!line)
    return 0;
  num = atoi (line);
  if (num < fc->base || num > fc->last)
    return 0;
  fc->messages[num - fc->base] = 1;
  return 0;
}

static int add_xover_line (char *line, void *c)
{
  unsigned int num, total;
  CONTEXT *ctx = fc->ctx;
  NNTP_DATA *data = (NNTP_DATA *)ctx->data;

  if (!line)
    return 0;

  if (ctx->msgcount >= ctx->hdrmax)
    mx_alloc_memory (ctx);
  ctx->hdrs[ctx->msgcount] = mutt_new_header ();
  ctx->hdrs[ctx->msgcount]->index = ctx->msgcount;

  nntp_parse_xover (ctx, line, ctx->hdrs[ctx->msgcount]);
  num = ctx->hdrs[ctx->msgcount]->article_num;

  if (num >= fc->first && num <= fc->last && fc->messages[num - fc->base])
  {
    ctx->msgcount++;
    if (num > data->lastLoaded)
      data->lastLoaded = num;
    num = num - fc->first + 1;
    total = fc->last - fc->first + 1;
    if (!ctx->quiet && fc->msg && ReadInc && (num % ReadInc == 0))
      mutt_message ("%s %d/%d", fc->msg, num, total);
  }
  else
    mutt_free_header (&ctx->hdrs[ctx->msgcount]); /* skip it */

  return 0;
}
#undef fc

static int nntp_fetch_headers (CONTEXT *ctx, unsigned int first,
	unsigned int last)
{
  char buf[HUGE_STRING];
  char *msg = _("Fetching message headers...");
  NNTP_DATA *nntp_data = ((NNTP_DATA *)ctx->data);
  int ret;
  int num;
  int oldmsgcount;
  unsigned int current;
  FILE *f;
  FETCH_CONTEXT fc;

  /* if empty group or nothing to do */
  if (!last || first > last)
    return 0;

  /* fetch list of articles */
  mutt_message _("Fetching list of articles...");
  fc.ctx = ctx;
  fc.base = first;
  fc.last = last;
  fc.messages = safe_calloc (last - first + 1, sizeof (unsigned short));
  snprintf (buf, sizeof (buf), "LISTGROUP %s\r\n", nntp_data->group);
  if (mutt_nntp_fetch (nntp_data, buf, NULL, nntp_fetch_numbers, &fc, 0) != 0)
  {
    mutt_error (_("LISTGROUP command failed: %s"), buf);
#ifdef DEBUG
    nntp_error ("nntp_fetch_headers()", buf);
#endif
    FREE (&fc.messages);
    return -1;
  }

  /* CACHE: must be loaded xover cache here */
  num = nntp_data->lastCached - first + 1;
  if (option (OPTNEWSCACHE) && nntp_data->cache && num > 0)
  {
    nntp_cache_expand (buf, nntp_data->cache);
    mutt_message _("Fetching headers from cache...");
    if ((f = safe_fopen (buf, "r")))
    {
      int r = 0;

      /* counting number of lines */
      while (fgets (buf, sizeof (buf), f) != NULL)
	r++;
      rewind (f);
      while (r > num && fgets (buf, sizeof (buf), f) != NULL)
	r--;
      oldmsgcount = ctx->msgcount;
      fc.first = first;
      fc.last = first + num - 1;
      fc.msg = NULL;
      while (fgets (buf, sizeof (buf), f) != NULL)
	add_xover_line (buf, &fc);
      fclose (f);
      nntp_data->lastLoaded = fc.last;
      first = fc.last + 1;
      if (ctx->msgcount > oldmsgcount)
	mx_update_context (ctx, ctx->msgcount - oldmsgcount);
    }
    else
      nntp_delete_cache (nntp_data);
  }
  num = last - first + 1;
  if (num <= 0)
  {
    FREE (&fc.messages);
    return 0;
  }

  /*
   * Without XOVER, we have to fetch each article header and parse
   * it.  With XOVER, we ask for all of them
   */
  mutt_message (msg);
  if (nntp_data->nserv->hasXOVER)
  {
    oldmsgcount = ctx->msgcount;
    fc.first = first;
    fc.last = last;
    fc.msg = msg;
    snprintf (buf, sizeof (buf), "XOVER %d-%d\r\n", first, last);
    ret = mutt_nntp_fetch (nntp_data, buf, NULL, add_xover_line, &fc, 0);
    if (ctx->msgcount > oldmsgcount)
      mx_update_context (ctx, ctx->msgcount - oldmsgcount);
    if (ret != 0)
    {
      mutt_error (_("XOVER command failed: %s"), buf);
#ifdef DEBUG
      nntp_error ("nntp_fetch_headers()", buf);
#endif
      FREE (&fc.messages);
      return -1;
    }
    /* fetched OK */
  }
  else
  for (current = first; current <= last; current++)
  {
    HEADER *h;

    ret = current - first + 1;
    mutt_message ("%s %d/%d", msg, ret, num);

    if (!fc.messages[current - fc.base])
      continue;

    if (ctx->msgcount >= ctx->hdrmax)
      mx_alloc_memory (ctx);
    h = ctx->hdrs[ctx->msgcount] = mutt_new_header ();
    h->index = ctx->msgcount;

    ret = nntp_read_header (ctx, NULL, current);
    if (ret == 0) /* Got article. Fetch next header */
    {
      nntp_get_status (ctx, h, NULL, h->article_num);
      ctx->msgcount++;
      mx_update_context (ctx, 1);
    }
    else
      mutt_free_header (&h); /* skip it */
    if (ret == -1)
    {
      FREE (&fc.messages);
      return -1;
    }

    if (current > nntp_data->lastLoaded)
      nntp_data->lastLoaded = current;
  }
  FREE (&fc.messages);
  nntp_data->lastLoaded = last;
  mutt_clear_error ();
  return 0;
}

/* 
 * currently, nntp "mailbox" is "newsgroup"
 */
int nntp_open_mailbox (CONTEXT *ctx)
{
  NNTP_DATA *nntp_data;
  NNTP_SERVER *serv;
  char buf[HUGE_STRING];
  char server[LONG_STRING];
  int count = 0;
  unsigned int first;
  ACCOUNT acct;

  if (nntp_parse_url (ctx->path, &acct, buf, sizeof (buf)) < 0 || !*buf)
  {
    mutt_error (_("%s is an invalid newsgroup specification!"), ctx->path);
    mutt_sleep (2);
    return -1;
  }

  server[0] = '\0';
  nntp_expand_path (server, sizeof (server), &acct);
  if (!(serv = mutt_select_newsserver (server)) || serv->status != NNTP_OK)
    return -1;

  CurrentNewsSrv = serv;

  /* create NNTP-specific state struct if nof found in list */
  if ((nntp_data = (NNTP_DATA *) hash_find (serv->newsgroups, buf)) == NULL)
  {
    nntp_data = safe_calloc (1, sizeof (NNTP_DATA) + strlen (buf) + 1);
    nntp_data->group = (char *) nntp_data + sizeof (NNTP_DATA);
    strcpy (nntp_data->group, buf);
    hash_insert (serv->newsgroups, nntp_data->group, nntp_data, 0);
    nntp_add_to_list (serv, nntp_data);
  }
  ctx->data = nntp_data;
  nntp_data->nserv = serv;

  mutt_message (_("Selecting %s..."), nntp_data->group);

  if (!nntp_data->desc)
  {
    nntp_get_desc (nntp_data, nntp_data->group, NULL);
    if (nntp_data->desc)
      nntp_save_cache_index (serv);
  }

  buf[0] = 0;
  if (mutt_nntp_query (nntp_data, buf, sizeof(buf)) < 0)
  {
#ifdef DEBUG
    nntp_error ("nntp_open_mailbox()", buf);
#endif
    return -1;
  }

  if (mutt_strncmp ("211", buf, 3))
  {
    LIST *l = serv->list;

    /* GROUP command failed */
    if (!mutt_strncmp ("411", buf, 3))
    {
      mutt_error (_("Newsgroup %s not found on server %s"),
		    nntp_data->group, serv->conn->account.host);

      /* CACHE: delete cache and line from .index */
      nntp_delete_cache (nntp_data);
      hash_delete (serv->newsgroups, nntp_data->group, NULL, nntp_delete_data);
      while (l && l->data != (void *) nntp_data) l = l->next;
      if (l)
	l->data = NULL;

      sleep (2);
    }

    return -1;
  }

  sscanf (buf + 4, "%d %u %u %s", &count, &nntp_data->firstMessage, 
	  &nntp_data->lastMessage, buf);

  nntp_data->deleted = 0;

  time (&serv->check_time);

  /*
   * Check for max adding context. If it is greater than $nntp_context,
   * strip off extra articles
   */
  first = nntp_data->firstMessage;
  if (NntpContext && nntp_data->lastMessage - first + 1 > NntpContext)
    first = nntp_data->lastMessage - NntpContext + 1;
  if (first)
    nntp_data->lastLoaded = first - 1;
  return nntp_fetch_headers (ctx, first, nntp_data->lastMessage);
}

int nntp_fetch_message (MESSAGE *msg, CONTEXT *ctx, int msgno)
{
  char buf[LONG_STRING];
  char path[_POSIX_PATH_MAX];
  NNTP_CACHE *cache;
  char *m = _("Fetching message...");
  int ret;

  /* see if we already have the message in our cache */
  cache = &((NNTP_DATA *) ctx->data)->acache[ctx->hdrs[msgno]->index % NNTP_CACHE_LEN];

  /* if everything is fine, assign msg->fp and return */
  if (cache->path && cache->index == ctx->hdrs[msgno]->index &&
      (msg->fp = fopen (cache->path, "r")))
    return 0;

  /* clear the previous entry */
  unlink (cache->path);
  free (cache->path);

  mutt_message (m);

  cache->index = ctx->hdrs[msgno]->index;
  mutt_mktemp (path);
  cache->path = safe_strdup (path);
  if (!(msg->fp = safe_fopen (path, "w+")))
  {
    FREE (&cache->path);
    return -1;
  }

  if (ctx->hdrs[msgno]->article_num == 0)
    snprintf (buf, sizeof (buf), "ARTICLE %s\r\n",
	  ctx->hdrs[msgno]->env->message_id);
  else
    snprintf (buf, sizeof (buf), "ARTICLE %d\r\n",
	  ctx->hdrs[msgno]->article_num);

  ret = mutt_nntp_fetch ((NNTP_DATA *)ctx->data, buf, m, nntp_read_tempfile,
	msg->fp, ctx->tagged);
  if (ret == 1)
  {
    mutt_error (_("Article %d not found on server"), 
	  ctx->hdrs[msgno]->article_num);
    dprint (1, (debugfile, "nntp_fetch_message: %s\n", buf));
  }

  if (ret)
  {
    fclose (msg->fp);
    unlink (path);
    FREE (&cache->path);
    return -1;
  }

  mutt_free_envelope (&ctx->hdrs[msgno]->env);
  ctx->hdrs[msgno]->env = mutt_read_rfc822_header (msg->fp, ctx->hdrs[msgno], 0, 0);
  /* fix content length */
  fseek(msg->fp, 0, SEEK_END);
  ctx->hdrs[msgno]->content->length = ftell (msg->fp) - 
                                        ctx->hdrs[msgno]->content->offset;

  /* this is called in mutt before the open which fetches the message, 
   * which is probably wrong, but we just call it again here to handle
   * the problem instead of fixing it.
   */
  mutt_parse_mime_message (ctx, ctx->hdrs[msgno]);

  /* These would normally be updated in mx_update_context(), but the 
   * full headers aren't parsed with XOVER, so the information wasn't
   * available then.
   */
#if defined(HAVE_PGP) || defined(HAVE_SMIME)
  ctx->hdrs[msgno]->security = crypt_query (ctx->hdrs[msgno]->content);
#endif /* HAVE_PGP || HAVE_SMIME */

  mutt_clear_error();
  rewind (msg->fp);

  return 0;
}

/* Post article */
int nntp_post (const char *msg) {
  char buf[LONG_STRING];
  size_t len;
  FILE *f;
  NNTP_DATA *nntp_data;

  if (Context && Context->magic == M_NNTP)
    nntp_data = (NNTP_DATA *)Context->data;
  else
  {
    if (!(CurrentNewsSrv = mutt_select_newsserver (NewsServer)) ||
	!CurrentNewsSrv->list || !CurrentNewsSrv->list->data)
    {
      mutt_error (_("Can't post article. No connection to news server."));
      return -1;
    }
    nntp_data = (NNTP_DATA *)CurrentNewsSrv->list->data;
  }

  if (!(f = safe_fopen (msg, "r")))
  {
    mutt_error (_("Can't post article. Unable to open %s"), msg);
    return -1;
  }

  strfcpy (buf, "POST\r\n", sizeof (buf));
  if (mutt_nntp_query (nntp_data, buf, sizeof (buf)) < 0)
  {
    mutt_error (_("Can't post article. Connection to %s lost."),
		nntp_data->nserv->conn->account.host);
    return -1;
  }
  if (buf[0] != '3')
  {
    mutt_error (_("Can't post article: %s"), buf);
    return -1;
  }

  buf[0] = '.';
  buf[1] = '\0';
  while (fgets (buf + 1, sizeof (buf) - 2, f) != NULL)
  {
    len = strlen (buf);
    if (buf[len - 1] == '\n')
    {
      buf[len - 1] = '\r';
      buf[len] = '\n';
      len++;
      buf[len] = '\0';
    }
    if (buf[1] == '.')
      mutt_socket_write_d (nntp_data->nserv->conn, buf, M_SOCK_LOG_HDR);
    else
      mutt_socket_write_d (nntp_data->nserv->conn, buf + 1, M_SOCK_LOG_HDR);
  }
  fclose (f);

  if (buf[strlen (buf) - 1] != '\n')
    mutt_socket_write_d (nntp_data->nserv->conn, "\r\n", M_SOCK_LOG_HDR);
  mutt_socket_write_d (nntp_data->nserv->conn, ".\r\n", M_SOCK_LOG_HDR);
  if (mutt_socket_readln (buf, sizeof (buf), nntp_data->nserv->conn) < 0)
  {
    mutt_error (_("Can't post article. Connection to %s lost."),
		nntp_data->nserv->conn->account.host);
    return -1;
  }
  if (buf[0] != '2')
  {
    mutt_error (_("Can't post article: %s"), buf);
    return -1;
  }

  return 0;
}

/* nntp_logout_all: close all open connections. */
void nntp_logout_all (void)
{
  char buf[LONG_STRING];
  CONNECTION* conn;
  CONNECTION* tmp;

  conn = mutt_socket_head ();

  while (conn)
  {
    tmp = conn;

    if (conn->account.type == M_ACCT_TYPE_NNTP)
    {
      mutt_message (_("Closing connection to %s..."), conn->account.host);
      mutt_socket_write (conn, "QUIT\r\n");
      mutt_socket_readln (buf, sizeof (buf), conn);
      mutt_clear_error ();
      mutt_socket_close (conn);

      mutt_socket_free (tmp);
    }

    conn = conn->next;
  }
}

static void nntp_free_acache (NNTP_DATA *data)
{
  int i;

  for (i = 0; i < NNTP_CACHE_LEN; i++)
  {
    if (data->acache[i].path)
    {
      unlink (data->acache[i].path);
      FREE (&data->acache[i].path);
    }
  }
}

void nntp_delete_data (void *p)
{
  NNTP_DATA *data = (NNTP_DATA *)p;

  if (!p)
    return;
  FREE (&data->entries);
  FREE (&data->desc);
  FREE (&data->cache);
  nntp_free_acache (data);
  FREE (p);
}

int nntp_sync_mailbox (CONTEXT *ctx)
{
  NNTP_DATA *data = ctx->data;

  /* CACHE: update cache and .index files */
  if ((option (OPTSAVEUNSUB) || data->subscribed))
    nntp_save_cache_group (ctx);
  nntp_free_acache (data);

  data->nserv->check_time = 0; /* next nntp_check_mailbox() will really check */
  return 0;
}

void nntp_fastclose_mailbox (CONTEXT *ctx)
{
  NNTP_DATA *data = (NNTP_DATA *) ctx->data, *tmp;

  if (!data)
    return;
  nntp_free_acache (data);
  if (!data->nserv || !data->nserv->newsgroups || !data->group)
    return;
  nntp_save_cache_index (data->nserv);
  if ((tmp = hash_find (data->nserv->newsgroups, data->group)) == NULL
	|| tmp != data)
    nntp_delete_data (data);
}

/* commit changes and terminate connection */
int nntp_close_mailbox (CONTEXT *ctx)
{
  if (!ctx)
    return -1;
  mutt_message _("Quitting newsgroup...");
  if (ctx->data)
  {
    NNTP_DATA *data = (NNTP_DATA *) ctx->data;
    int ret;

    if (data->nserv && data->nserv->conn && ctx->unread)
    {
      ret = query_quadoption (OPT_CATCHUP, _("Mark all articles read?"));
      if (ret == M_YES)
	mutt_newsgroup_catchup (data->nserv, data->group);
      else if (ret < 0)
	return -1;
    }
  }
  nntp_sync_mailbox (ctx);
  if (ctx->data && ((NNTP_DATA *)ctx->data)->nserv)
  {
    NNTP_SERVER *news;

    news = ((NNTP_DATA *)ctx->data)->nserv;
    newsrc_gen_entries (ctx);
    ((NNTP_DATA *)ctx->data)->unread = ctx->unread;
    mutt_newsrc_update (news);
  }
  mutt_clear_error();
  return 0;
}

/* use the GROUP command to poll for new mail */
static int _nntp_check_mailbox (CONTEXT *ctx, NNTP_DATA *nntp_data)
{
  char buf[LONG_STRING];
  int count = 0;

  if (nntp_data->nserv->check_time + NewsPollTimeout > time (NULL))
    return 0;

  buf[0] = 0;
  if (mutt_nntp_query (nntp_data, buf, sizeof (buf)) < 0)
  {
#ifdef DEBUG
    nntp_error ("nntp_check_mailbox()", buf);
#endif
    return -1;
  }
  if (mutt_strncmp ("211", buf, 3))
  {
    buf[0] = 0;
    if (mutt_nntp_query (nntp_data, buf, sizeof (buf)) < 0)
    {
#ifdef DEBUG
      nntp_error ("nntp_check_mailbox()", buf);
#endif
      return -1;
    }
  }
  if (!mutt_strncmp ("211", buf, 3))
  {
    int first;
    int last;

    sscanf (buf + 4, "%d %d %d", &count, &first, &last);
    nntp_data->firstMessage = first;
    nntp_data->lastMessage = last;
    if (ctx && last > nntp_data->lastLoaded)
    {
      nntp_fetch_headers (ctx, nntp_data->lastLoaded + 1, last);
      time (&nntp_data->nserv->check_time);
      return 1;
    }
    if (!last || (!nntp_data->rc && !nntp_data->lastCached))
      nntp_data->unread = count;
    else
      mutt_newsgroup_stat (nntp_data);
    /* active was renumbered? */
    if (last < nntp_data->lastLoaded)
    {
      if (!nntp_data->max)
      {
	nntp_data->entries = safe_calloc (5, sizeof (NEWSRC_ENTRY));
	nntp_data->max = 5;
      }
      nntp_data->lastCached = 0;
      nntp_data->num = 1;
      nntp_data->entries[0].first = 1;
      nntp_data->entries[0].last = 0;
    }
  }

  time (&nntp_data->nserv->check_time);
  return 0;
}

int nntp_check_mailbox (CONTEXT *ctx)
{
  return _nntp_check_mailbox (ctx, (NNTP_DATA *)ctx->data);
}

static int add_group (char *buf, void *serv)
{
#define s ((NNTP_SERVER *) serv)
  char group[LONG_STRING], mod, desc[HUGE_STRING];
  int first, last;
  NNTP_DATA *nntp_data;
  static int n = 0;

  _checked = n;	/* _checked have N, where N = number of groups */
  if (!buf)	/* at EOF must be zerouth */
    n = 0;

  if (!s || !buf)
    return 0;

  *desc = 0;
  sscanf (buf, "%s %d %d %c %[^\n]", group, &last, &first, &mod, desc);
  if (!group)
    return 0;
  if ((nntp_data = (NNTP_DATA *) hash_find (s->newsgroups, group)) == NULL)
  {
    n++;
    nntp_data = safe_calloc (1, sizeof (NNTP_DATA) + strlen (group) + 1);
    nntp_data->group = (char *) nntp_data + sizeof (NNTP_DATA);
    strcpy (nntp_data->group, group);
    nntp_data->nserv = s;
    if (s->newsgroups->nelem < s->newsgroups->curnelem * 2)
      s->newsgroups = hash_resize (s->newsgroups, s->newsgroups->nelem * 2);
    hash_insert (s->newsgroups, nntp_data->group, nntp_data, 0);
    nntp_add_to_list (s, nntp_data);
  }
  nntp_data->deleted = 0;
  nntp_data->firstMessage = first;
  nntp_data->lastMessage = last;
  if (mod == 'y')
    nntp_data->allowed = 1;
  else
    nntp_data->allowed = 0;
  if (nntp_data->desc)
    FREE (&nntp_data->desc);
  if (*desc)
    nntp_data->desc = safe_strdup (desc);
  if (nntp_data->rc || nntp_data->lastCached)
    mutt_newsgroup_stat (nntp_data);
  else if (nntp_data->lastMessage &&
	nntp_data->firstMessage <= nntp_data->lastMessage)
    nntp_data->unread = nntp_data->lastMessage - nntp_data->firstMessage + 1;
  else
    nntp_data->unread = 0;

  return 0;
#undef s
}

int nntp_check_newgroups (NNTP_SERVER *serv, int force)
{
  char buf[LONG_STRING];
  NNTP_DATA nntp_data;
  LIST *l;
  LIST emp;
  time_t now;
  struct tm *t;

  if (!serv || !serv->newgroups_time)
    return -1;

  if (nntp_open_connection (serv) < 0)
    return -1;

  /* check subscribed groups for new news */
  if (option (OPTSHOWNEWNEWS))
  {
    mutt_message _("Checking for new messages...");
    for (l = serv->list; l; l = l->next)
    {
      serv->check_time = 0;	/* really check! */
      if (l->data && ((NNTP_DATA *) l->data)->subscribed)
	_nntp_check_mailbox (NULL, (NNTP_DATA *) l->data);
    }
  }
  else if (!force)
    return 0;

  mutt_message _("Checking for new newsgroups...");
  now = serv->newgroups_time;
  time (&serv->newgroups_time);
  t = gmtime (&now);
  snprintf (buf, sizeof (buf), "NEWGROUPS %02d%02d%02d %02d%02d%02d GMT\r\n",
	(t->tm_year % 100), t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min,
	t->tm_sec);
  nntp_data.nserv = serv;
  if (Context && Context->magic == M_NNTP)
    nntp_data.group = ((NNTP_DATA *)Context->data)->group;
  else
    nntp_data.group = NULL;
  l = serv->tail;
  if (mutt_nntp_fetch (&nntp_data, buf, _("Adding new newsgroups..."),
	add_group, serv, 0) != 0)
  {
#ifdef DEBUG
    nntp_error ("nntp_check_newgroups()", buf);
#endif
    return -1;
  }

  mutt_message _("Loading descriptions...");
  if (l)
    emp.next = l->next;
  else
    emp.next = serv->list;
  l = &emp;
  while (l->next)
  {
    l = l->next;
    ((NNTP_DATA *) l->data)->new = 1;
    nntp_get_desc ((NNTP_DATA *) l->data, ((NNTP_DATA *) l->data)->group, NULL);
  }
  if (emp.next)
    nntp_save_cache_index (serv);
  mutt_clear_error ();
  return _checked;
}

/* Load list of all newsgroups from cache ALL */
int nntp_get_cache_all (NNTP_SERVER *serv)
{
  char buf[HUGE_STRING];
  FILE *f;

  nntp_cache_expand (buf, serv->cache);
  if ((f = safe_fopen (buf, "r")))
  {
    int i = 0;

    while (fgets (buf, sizeof(buf), f) != NULL)
    {
      if (ReadInc && (i % ReadInc == 0))
	mutt_message (_("Loading list from cache... %d"), i);
      add_group (buf, serv);
      i++;
    }
    add_group (NULL, NULL);
    fclose (f);
    mutt_clear_error ();
    return 0;
  }
  else
  {
    FREE (&serv->cache);
    return -1;
  }
}

/* Load list of all newsgroups from active */
int nntp_get_active (NNTP_SERVER *serv)
{
  char msg[SHORT_STRING];
  NNTP_DATA nntp_data;
  LIST *tmp;

  if (nntp_open_connection (serv) < 0)
    return -1;

  snprintf (msg, sizeof(msg), _("Loading list of all newsgroups on server %s..."),
		serv->conn->account.host);
  mutt_message (msg);
  time (&serv->newgroups_time);
  nntp_data.nserv = serv;
  nntp_data.group = NULL;

  if (mutt_nntp_fetch (&nntp_data, "LIST\r\n", msg, add_group, serv, 0) < 0)
  {
#ifdef DEBUG
    nntp_error ("nntp_get_active()", "LIST\r\n");
#endif
    return -1;
  }

  strfcpy (msg, _("Loading descriptions..."), sizeof (msg));
  mutt_message (msg);
  nntp_get_desc (&nntp_data, "*", msg);

  for (tmp = serv->list; tmp; tmp = tmp->next)
  {
    NNTP_DATA *data = (NNTP_DATA *)tmp->data;

    if (data && data->deleted && !data->rc)
    {
      nntp_delete_cache (data);
      hash_delete (serv->newsgroups, data->group, NULL, nntp_delete_data);
      tmp->data = NULL;
    }
  }
  nntp_save_cache_index (serv);

  mutt_clear_error ();
  return _checked;
}

/*
 * returns -1 if error ocurred while retrieving header,
 * number of articles which ones exist in context on success.
 */
int nntp_check_msgid (CONTEXT *ctx, const char *msgid)
{
  int ret;

  /* if msgid is already in context, don't reload them */
  if (hash_find (ctx->id_hash, msgid))
    return 1;
  if (ctx->msgcount == ctx->hdrmax)
    mx_alloc_memory (ctx);
  ctx->hdrs[ctx->msgcount] = mutt_new_header ();
  ctx->hdrs[ctx->msgcount]->index = ctx->msgcount;
  
  mutt_message (_("Fetching %s from server..."), msgid);
  ret = nntp_read_header (ctx, msgid, 0);
  /* since nntp_read_header() may set read flag, we must reset it */
  ctx->hdrs[ctx->msgcount]->read = 0;
  if (ret != 0)
    mutt_free_header (&ctx->hdrs[ctx->msgcount]);
  else
  {
    ctx->msgcount++;
    mx_update_context (ctx, 1);
    ctx->changed = 1;
  }
  return ret;
}

typedef struct
{
  CONTEXT *ctx;
  unsigned int num;
  unsigned int max;
  unsigned int *child;
} CHILD_CONTEXT;

static int check_children (char *s, void *c)
{
#define cc ((CHILD_CONTEXT *) c)
  unsigned int i, n;

  if (!s || (n = atoi (s)) == 0)
    return 0;
  for (i = 0; i < cc->ctx->msgcount; i++)
    if (cc->ctx->hdrs[i]->article_num == n)
      return 0;
  if (cc->num >= cc->max)
    safe_realloc (&cc->child, sizeof (unsigned int) * (cc->max += 25));
  cc->child[cc->num++] = n;

  return 0;
#undef cc
}

int nntp_check_children (CONTEXT *ctx, const char *msgid)
{
  NNTP_DATA *nntp_data = (NNTP_DATA *)ctx->data;
  char buf[STRING];
  int i, ret = 0, tmp = 0;
  CHILD_CONTEXT cc;

  if (!nntp_data || !nntp_data->nserv || !nntp_data->nserv->conn ||
	!nntp_data->nserv->conn->account.host)
    return -1;
  if (nntp_data->firstMessage > nntp_data->lastLoaded)
    return 0;
  if (!nntp_data->nserv->hasXPAT)
  {
    mutt_error (_("Server %s does not support this operation!"),
	  nntp_data->nserv->conn->account.host);
    return -1;
  }

  snprintf (buf, sizeof (buf), "XPAT References %d-%d *%s*\r\n", 
	nntp_data->firstMessage, nntp_data->lastLoaded, msgid);

  cc.ctx = ctx;
  cc.num = 0;
  cc.max = 25;
  cc.child = safe_malloc (sizeof (unsigned int) * 25);
  if (mutt_nntp_fetch (nntp_data, buf, NULL, check_children, &cc, 0))
  {
    FREE (&cc.child);
    return -1;
  }
  /* dont try to read the xover cache. check_children() already
   * made sure that we dont have the article, so we need to visit
   * the server. Reading the cache at this point is also bad
   * because it would duplicate messages */
  if (option (OPTNEWSCACHE))
  {
    tmp++;
    unset_option (OPTNEWSCACHE);
  }
  for (i = 0; i < cc.num; i++)
  {
    if ((ret = nntp_fetch_headers (ctx, cc.child[i], cc.child[i])))
      break;
    if (ctx->msgcount &&
	  ctx->hdrs[ctx->msgcount - 1]->article_num == cc.child[i])
      ctx->hdrs[ctx->msgcount - 1]->read = 0;
  }
  if (tmp)
    set_option (OPTNEWSCACHE);
  FREE (&cc.child);
  return ret;
}
