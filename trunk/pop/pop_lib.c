/*
 * Copyright notice from original mutt:
 * Copyright (C) 2000-2003 Vsevolod Volkov <vvv@mutt.org.ua>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "mutt.h"
#include "ascii.h"
#include "mx.h"
#include "url.h"
#include "pop.h"
#if defined (USE_SSL) || defined (USE_GNUTLS)
# include "mutt_ssl.h"
#endif

#include "lib/mem.h"
#include "lib/intl.h"
#include "lib/debug.h"
#include "lib/str.h"

#include <string.h>
#include <unistd.h>
#include <ctype.h>

/* given an POP mailbox name, return host, port, username and password */
int pop_parse_path (const char *path, ACCOUNT * acct)
{
  ciss_url_t url;
  char *c;
  int ret = -1;

  /* Defaults */
  acct->flags = 0;
  acct->port = POP_PORT;
  acct->type = M_ACCT_TYPE_POP;

  c = str_dup (path);
  url_parse_ciss (&url, c);

  if (url.scheme == U_POP || url.scheme == U_POPS) {
    if (url.scheme == U_POPS) {
      acct->flags |= M_ACCT_SSL;
      acct->port = POP_SSL_PORT;
    }

    if ((!url.path || !*url.path) && mutt_account_fromurl (acct, &url) == 0)
      ret = 0;
  }

  mem_free (&c);
  return ret;
}

/* Copy error message to err_msg buffer */
void pop_error (POP_DATA * pop_data, char *msg)
{
  char *t, *c, *c2;

  t = strchr (pop_data->err_msg, '\0');
  c = msg;

  if (!str_ncmp (msg, "-ERR ", 5)) {
    c2 = msg + 5;
    SKIPWS (c2);

    if (*c2)
      c = c2;
  }

  strfcpy (t, c, sizeof (pop_data->err_msg) - strlen (pop_data->err_msg));
  str_skip_trailws (pop_data->err_msg);
}

/* Parse CAPA output */
static int fetch_capa (char *line, void *data)
{
  POP_DATA *pop_data = (POP_DATA *) data;
  char *c;

  if (!ascii_strncasecmp (line, "SASL", 4)) {
    mem_free (&pop_data->auth_list);
    c = line + 4;
    SKIPWS (c);
    pop_data->auth_list = str_dup (c);
  }

  else if (!ascii_strncasecmp (line, "STLS", 4))
    pop_data->cmd_stls = CMD_AVAILABLE;

  else if (!ascii_strncasecmp (line, "USER", 4))
    pop_data->cmd_user = CMD_AVAILABLE;

  else if (!ascii_strncasecmp (line, "UIDL", 4))
    pop_data->cmd_uidl = CMD_AVAILABLE;

  else if (!ascii_strncasecmp (line, "TOP", 3))
    pop_data->cmd_top = CMD_AVAILABLE;

  return 0;
}

/* Fetch list of the authentication mechanisms */
static int fetch_auth (char *line, void *data)
{
  POP_DATA *pop_data = (POP_DATA *) data;

  if (!pop_data->auth_list) {
    pop_data->auth_list = mem_malloc (strlen (line) + 1);
    *pop_data->auth_list = '\0';
  }
  else {
    mem_realloc (&pop_data->auth_list,
                  strlen (pop_data->auth_list) + strlen (line) + 2);
    strcat (pop_data->auth_list, " ");  /* __STRCAT_CHECKED__ */
  }
  strcat (pop_data->auth_list, line);   /* __STRCAT_CHECKED__ */

  return 0;
}

/*
 * Get capabilities
 *  0 - successful,
 * -1 - conection lost,
 * -2 - execution error.
*/
static pop_query_status pop_capabilities (POP_DATA * pop_data, int mode)
{
  char buf[LONG_STRING];

  /* don't check capabilities on reconnect */
  if (pop_data->capabilities)
    return 0;

  /* init capabilities */
  if (mode == 0) {
    pop_data->cmd_capa = CMD_NOT_AVAILABLE;
    pop_data->cmd_stls = CMD_NOT_AVAILABLE;
    pop_data->cmd_user = CMD_NOT_AVAILABLE;
    pop_data->cmd_uidl = CMD_NOT_AVAILABLE;
    pop_data->cmd_top = CMD_NOT_AVAILABLE;
    pop_data->resp_codes = 0;
    pop_data->expire = 1;
    pop_data->login_delay = 0;
    mem_free (&pop_data->auth_list);
  }

  /* Execute CAPA command */
  if (mode == 0 || pop_data->cmd_capa != CMD_NOT_AVAILABLE) {
    strfcpy (buf, "CAPA\r\n", sizeof (buf));
    switch (pop_fetch_data (pop_data, buf, NULL, fetch_capa, pop_data)) {
    case PQ_OK:
      {
        pop_data->cmd_capa = CMD_AVAILABLE;
        break;
      }
    case PFD_FUNCT_ERROR:
    case PQ_ERR:
      {
        pop_data->cmd_capa = CMD_NOT_AVAILABLE;
        break;
      }
    case PQ_NOT_CONNECTED:
      return PQ_NOT_CONNECTED;
    }
  }

  /* CAPA not supported, use defaults */
  if (mode == 0 && pop_data->cmd_capa == CMD_NOT_AVAILABLE) {
    pop_data->cmd_user = CMD_UNKNOWN;
    pop_data->cmd_uidl = CMD_UNKNOWN;
    pop_data->cmd_top = CMD_UNKNOWN;

    strfcpy (buf, "AUTH\r\n", sizeof (buf));
    if (pop_fetch_data (pop_data, buf, NULL, fetch_auth, pop_data) == PQ_NOT_CONNECTED)
      return PQ_NOT_CONNECTED;
  }

  /* Check capabilities */
  if (mode == 2) {
    char *msg = NULL;

    if (!pop_data->expire)
      msg = _("Unable to leave messages on server.");
    if (pop_data->cmd_top == CMD_NOT_AVAILABLE)
      msg = _("Command TOP is not supported by server.");
    if (pop_data->cmd_uidl == CMD_NOT_AVAILABLE)
      msg = _("Command UIDL is not supported by server.");
    if (msg && pop_data->cmd_capa != CMD_AVAILABLE) {
      mutt_error (msg);
      return PQ_ERR;
    }
    pop_data->capabilities = 1;
  }

  return PQ_OK;
}

/*
 * Open connection
 *  0 - successful,
 * -1 - conection lost,
 * -2 - invalid response.
*/
pop_query_status pop_connect (POP_DATA * pop_data)
{
  char buf[LONG_STRING];

  pop_data->status = POP_NONE;
  if (mutt_socket_open (pop_data->conn) < 0 ||
      mutt_socket_readln (buf, sizeof (buf), pop_data->conn) < 0) {
    mutt_error (_("Error connecting to server: %s"),
                pop_data->conn->account.host);
    return PQ_NOT_CONNECTED;
  }

  pop_data->status = POP_CONNECTED;

  if (str_ncmp (buf, "+OK", 3)) {
    *pop_data->err_msg = '\0';
    pop_error (pop_data, buf);
    mutt_error ("%s", pop_data->err_msg);
    return PQ_ERR;
  }

  pop_apop_timestamp (pop_data, buf);

  return PQ_OK;
}

/*
 * Open connection and authenticate
 *  0 - successful,
 * -1 - conection lost,
 * -2 - invalid command or execution error,
 * -3 - authentication canceled.
*/
pop_query_status pop_open_connection (POP_DATA * pop_data)
{
  pop_query_status ret;
  unsigned int n, size;
  char buf[LONG_STRING];

  ret = pop_connect (pop_data);
  if (ret != PQ_OK) {
    mutt_sleep (2);
    return ret;
  }

  ret = pop_capabilities (pop_data, 0);
  if (ret == PQ_NOT_CONNECTED)
    goto err_conn;
  if (ret == PQ_ERR) {
    mutt_sleep (2);
    return PQ_ERR;
  }

#if (defined(USE_SSL) || defined(USE_GNUTLS))
  /* Attempt STLS if available and desired. */
  if (!pop_data->conn->ssf && (pop_data->cmd_stls || option(OPTSSLFORCETLS))) {
    if (option (OPTSSLFORCETLS))
      pop_data->use_stls = 2;
    if (pop_data->use_stls == 0) {
      ret = query_quadoption (OPT_SSLSTARTTLS,
                              _("Secure connection with TLS?"));
      if (ret == -1)
        return PQ_ERR;
      pop_data->use_stls = 1;
      if (ret == M_YES)
        pop_data->use_stls = 2;
    }
    if (pop_data->use_stls == 2) {
      strfcpy (buf, "STLS\r\n", sizeof (buf));
      ret = pop_query (pop_data, buf, sizeof (buf));
      if (ret == PQ_NOT_CONNECTED)
        goto err_conn;
      if (ret != PQ_OK) {
        mutt_error ("%s", pop_data->err_msg);
        mutt_sleep (2);
      }
#if defined (USE_SSL) || defined (USE_GNUTLS)
      else if (mutt_ssl_starttls (pop_data->conn))
#endif
      {
        mutt_error (_("Could not negotiate TLS connection"));
        mutt_sleep (2);
        return PQ_ERR;
      }
      else {
        /* recheck capabilities after STLS completes */
        ret = pop_capabilities (pop_data, 1);
        if (ret == PQ_NOT_CONNECTED)
          goto err_conn;
        if (ret == PQ_ERR) {
          mutt_sleep (2);
          return PQ_ERR;
        }
      }
    }
  }

  if (option(OPTSSLFORCETLS) && !pop_data->conn->ssf) {
    mutt_error _("Encrypted connection unavailable");
    mutt_sleep (1);
    return -2;
  }
#endif

  ret = pop_authenticate (pop_data);
  if (ret == PQ_NOT_CONNECTED)
    goto err_conn;
  if (ret == PFD_FUNCT_ERROR)
    mutt_clear_error ();
  if (ret != PQ_OK)
    return ret;

  /* recheck capabilities after authentication */
  ret = pop_capabilities (pop_data, 2);
  if (ret == PQ_NOT_CONNECTED)
    goto err_conn;
  if (ret == PQ_ERR) {
    mutt_sleep (2);
    return PQ_ERR;
  }

  /* get total size of mailbox */
  strfcpy (buf, "STAT\r\n", sizeof (buf));
  ret = pop_query (pop_data, buf, sizeof (buf));
  if (ret == PQ_NOT_CONNECTED)
    goto err_conn;
  if (ret == PQ_ERR) {
    mutt_error ("%s", pop_data->err_msg);
    mutt_sleep (2);
    return ret;
  }

  sscanf (buf, "+OK %u %u", &n, &size);
  pop_data->size = size;
  return PQ_OK;

err_conn:
  pop_data->status = POP_DISCONNECTED;
  mutt_error _("Server closed connection!");

  mutt_sleep (2);
  return PQ_NOT_CONNECTED;
}

/* logout from POP server */
void pop_logout (CONTEXT * ctx)
{
  pop_query_status ret = 0;
  char buf[LONG_STRING];
  POP_DATA *pop_data = (POP_DATA *) ctx->data;

  if (pop_data->status == POP_CONNECTED) {
    mutt_message _("Closing connection to POP server...");

    if (ctx->readonly) {
      strfcpy (buf, "RSET\r\n", sizeof (buf));
      ret = pop_query (pop_data, buf, sizeof (buf));
    }

    if (ret != PQ_NOT_CONNECTED) {
      strfcpy (buf, "QUIT\r\n", sizeof (buf));
      pop_query (pop_data, buf, sizeof (buf));
    }

    mutt_clear_error ();
  }

  pop_data->status = POP_DISCONNECTED;
  return;
}

/*
 * Send data from buffer and receive answer to the same buffer
 *  0 - successful,
 * -1 - conection lost,
 * -2 - invalid command or execution error.
*/
pop_query_status pop_query_d (POP_DATA * pop_data, char *buf, size_t buflen, char *msg)
{
  int dbg = M_SOCK_LOG_CMD;
  char *c;

  if (pop_data->status != POP_CONNECTED)
    return PQ_NOT_CONNECTED;

#ifdef DEBUG
  /* print msg instaed of real command */
  if (msg) {
    dbg = M_SOCK_LOG_FULL;
    debug_print (M_SOCK_LOG_CMD, ("> %s", msg));
  }
#endif

  mutt_socket_write_d (pop_data->conn, buf, dbg);

  c = strpbrk (buf, " \r\n");
  *c = '\0';
  snprintf (pop_data->err_msg, sizeof (pop_data->err_msg), "%s: ", buf);

  if (mutt_socket_readln (buf, buflen, pop_data->conn) < 0) {
    pop_data->status = POP_DISCONNECTED;
    return PQ_NOT_CONNECTED;
  }
  if (!str_ncmp (buf, "+OK", 3))
    return PQ_OK;

  pop_error (pop_data, buf);
  return PQ_ERR;
}

/*
 * This function calls  funct(*line, *data)  for each received line,
 * funct(NULL, *data)  if  rewind(*data)  needs, exits when fail or done.
 * Returned codes:
 *  0 - successful,
 * -1 - conection lost,
 * -2 - invalid command or execution error,
 * -3 - error in funct(*line, *data)
 */
pop_query_status pop_fetch_data (POP_DATA * pop_data, char *query, char *msg,
                    int (*funct) (char *, void *), void *data)
{
  char buf[LONG_STRING];
  char *inbuf;
  char *p;
  pop_query_status ret;
  int chunk, line = 0;
  size_t lenbuf = 0;

  strfcpy (buf, query, sizeof (buf));
  ret = pop_query (pop_data, buf, sizeof (buf));
  if (ret != PQ_OK)
    return ret;

  inbuf = mem_malloc (sizeof (buf));

  FOREVER {
    chunk =
      mutt_socket_readln_d (buf, sizeof (buf), pop_data->conn,
                            M_SOCK_LOG_HDR);
    if (chunk < 0) {
      pop_data->status = POP_DISCONNECTED;
      ret = PQ_NOT_CONNECTED;
      break;
    }

    p = buf;
    if (!lenbuf && buf[0] == '.') {
      if (buf[1] != '.')
        break;
      p++;
    }

    strfcpy (inbuf + lenbuf, p, sizeof (buf));

    if (chunk >= sizeof (buf)) {
      lenbuf += strlen (p);
    }
    else {
      line++;
      if (msg && ReadInc && (line % ReadInc == 0))
        mutt_message ("%s %d", msg, line);
      if (ret == 0 && funct (inbuf, data) < 0)
        ret = PFD_FUNCT_ERROR;
      lenbuf = 0;
    }

    mem_realloc (&inbuf, lenbuf + sizeof (buf));
  }

  mem_free (&inbuf);
  return ret;
}

/* find message with this UIDL and set refno */
static int check_uidl (char *line, void *data)
{
  int i;
  unsigned int index;
  CONTEXT *ctx = (CONTEXT *) data;

  sscanf (line, "%u %s", &index, line);
  for (i = 0; i < ctx->msgcount; i++) {
    if (!str_cmp (ctx->hdrs[i]->data, line)) {
      ctx->hdrs[i]->refno = index;
      break;
    }
  }

  return 0;
}

/* reconnect and verify indexes if connection was lost */
pop_query_status pop_reconnect (CONTEXT * ctx)
{
  pop_query_status ret;
  POP_DATA *pop_data = (POP_DATA *) ctx->data;

  if (pop_data->status == POP_CONNECTED)
    return PQ_OK;
  if (pop_data->status == POP_BYE)
    return PQ_NOT_CONNECTED;

  FOREVER {
    mutt_socket_close (pop_data->conn);

    ret = pop_open_connection (pop_data);
    if (ret == PQ_OK) {
      char *msg = _("Verifying message indexes...");
      int i;

      for (i = 0; i < ctx->msgcount; i++)
        ctx->hdrs[i]->refno = -1;

      mutt_message (msg);

      ret = pop_fetch_data (pop_data, "UIDL\r\n", msg, check_uidl, ctx);
      if (ret == PQ_ERR) {
        mutt_error ("%s", pop_data->err_msg);
        mutt_sleep (2);
      }
    }
    if (ret == PQ_OK)
      return PQ_OK;

    pop_logout (ctx);

    if (ret == PQ_ERR)
      return PQ_NOT_CONNECTED;

    if (query_quadoption (OPT_POPRECONNECT,
                          _("Connection lost. Reconnect to POP server?")) !=
        M_YES)
      return PQ_NOT_CONNECTED;
  }
}
