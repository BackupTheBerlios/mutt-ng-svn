/*
 * Copyright notice from original mutt:
 * Copyright (C) 2000-2001 Vsevolod Volkov <vvv@mutt.org.ua>
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
#include "md5.h"
#include "pop.h"

#include "lib/mem.h"
#include "lib/intl.h"
#include "lib/debug.h"

#include <string.h>
#include <unistd.h>

#ifdef USE_SASL
#ifdef USE_SASL2
#include <sasl/sasl.h>
#include <sasl/saslutil.h>
#else
#include <sasl.h>
#include <saslutil.h>
#endif

#include "mutt_sasl.h"
#endif

#ifdef USE_SASL
/* SASL authenticator */
static pop_auth_res_t pop_auth_sasl (POP_DATA * pop_data, const char *method)
{
  sasl_conn_t *saslconn;
  sasl_interact_t *interaction = NULL;
  int rc;
  char buf[LONG_STRING];
  char inbuf[LONG_STRING];
  const char *mech;

#ifdef USE_SASL2
  const char *pc = NULL;
#else
  char *pc = NULL;
#endif
  unsigned int len, olen;
  unsigned char client_start;

  if (mutt_sasl_client_new (pop_data->conn, &saslconn) < 0) {
    debug_print (1, ("Error allocating SASL connection.\n"));
    return POP_A_FAILURE;
  }

  if (!method)
    method = pop_data->auth_list;

  FOREVER {
#ifdef USE_SASL2
    rc =
      sasl_client_start (saslconn, method, &interaction, &pc, &olen, &mech);
#else
    rc = sasl_client_start (saslconn, method, NULL,
                            &interaction, &pc, &olen, &mech);
#endif
    if (rc != SASL_INTERACT)
      break;
    mutt_sasl_interact (interaction);
  }

  if (rc != SASL_OK && rc != SASL_CONTINUE) {
    debug_print (1, ("Failure starting authentication exchange. No shared mechanisms?\n"));

    /* SASL doesn't support suggested mechanisms, so fall back */
    return POP_A_UNAVAIL;
  }

  client_start = (olen > 0);

  mutt_message _("Authenticating (SASL)...");

  snprintf (buf, sizeof (buf), "AUTH %s", mech);
  olen = strlen (buf);

  /* looping protocol */
  FOREVER {
    strfcpy (buf + olen, "\r\n", sizeof (buf) - olen);
    mutt_socket_write (pop_data->conn, buf);
    if (mutt_socket_readln (inbuf, sizeof (inbuf), pop_data->conn) < 0) {
      sasl_dispose (&saslconn);
      pop_data->status = POP_DISCONNECTED;
      return POP_A_SOCKET;
    }

    if (rc != SASL_CONTINUE)
      break;

#ifdef USE_SASL2
    if (!str_ncmp (inbuf, "+ ", 2)
        && sasl_decode64 (inbuf, strlen (inbuf), buf, LONG_STRING - 1,
                          &len) != SASL_OK)
#else
    if (!str_ncmp (inbuf, "+ ", 2)
        && sasl_decode64 (inbuf, strlen (inbuf), buf, &len) != SASL_OK)
#endif
    {
      debug_print (1, ("error base64-decoding server response.\n"));
      goto bail;
    }

    if (!client_start)
      FOREVER {
      rc = sasl_client_step (saslconn, buf, len, &interaction, &pc, &olen);
      if (rc != SASL_INTERACT)
        break;
      mutt_sasl_interact (interaction);
      }
    else
      client_start = 0;

    if (rc != SASL_CONTINUE && (olen == 0 || rc != SASL_OK))
      break;

    /* send out response, or line break if none needed */
    if (pc) {
      if (sasl_encode64 (pc, olen, buf, sizeof (buf), &olen) != SASL_OK) {
        debug_print (1, ("error base64-encoding client response.\n"));
        goto bail;
      }

      /* sasl_client_st(art|ep) allocate pc with malloc, expect me to 
       * free it */
#ifndef USE_SASL2
      FREE (&pc);
#endif
    }
  }

  if (rc != SASL_OK)
    goto bail;

  if (!str_ncmp (inbuf, "+OK", 3)) {
    mutt_sasl_setup_conn (pop_data->conn, saslconn);
    return POP_A_SUCCESS;
  }

bail:
  sasl_dispose (&saslconn);

  /* terminate SASL sessoin if the last responce is not +OK nor -ERR */
  if (!str_ncmp (inbuf, "+ ", 2)) {
    snprintf (buf, sizeof (buf), "*\r\n");
    if (pop_query (pop_data, buf, sizeof (buf)) == PQ_NOT_CONNECTED)
      return POP_A_SOCKET;
  }

  mutt_error _("SASL authentication failed.");

  mutt_sleep (2);

  return POP_A_FAILURE;
}
#endif

/* Get the server timestamp for APOP authentication */
void pop_apop_timestamp (POP_DATA * pop_data, char *buf)
{
  char *p1, *p2;

  FREE (&pop_data->timestamp);

  if ((p1 = strchr (buf, '<')) && (p2 = strchr (p1, '>'))) {
    p2[1] = '\0';
    pop_data->timestamp = str_dup (p1);
  }
}

/* APOP authenticator */
static pop_auth_res_t pop_auth_apop (POP_DATA * pop_data, const char *method)
{
  MD5_CTX mdContext;
  unsigned char digest[16];
  char hash[33];
  char buf[LONG_STRING];
  int i;

  if (!pop_data->timestamp)
    return POP_A_UNAVAIL;

  mutt_message _("Authenticating (APOP)...");

  /* Compute the authentication hash to send to the server */
  MD5Init (&mdContext);
  MD5Update (&mdContext, (unsigned char *) pop_data->timestamp,
             strlen (pop_data->timestamp));
  MD5Update (&mdContext, (unsigned char *) pop_data->conn->account.pass,
             strlen (pop_data->conn->account.pass));
  MD5Final (digest, &mdContext);

  for (i = 0; i < sizeof (digest); i++)
    sprintf (hash + 2 * i, "%02x", digest[i]);

  /* Send APOP command to server */
  snprintf (buf, sizeof (buf), "APOP %s %s\r\n", pop_data->conn->account.user,
            hash);

  switch (pop_query (pop_data, buf, sizeof (buf))) {
  case PQ_OK:
    return POP_A_SUCCESS;
  case PQ_NOT_CONNECTED:
    return POP_A_SOCKET;
  }

  mutt_error _("APOP authentication failed.");

  mutt_sleep (2);

  return POP_A_FAILURE;
}

/* USER authenticator */
static pop_auth_res_t pop_auth_user (POP_DATA * pop_data, const char *method)
{
  char buf[LONG_STRING];
  pop_query_status ret;

  if (pop_data->cmd_user == CMD_NOT_AVAILABLE)
    return POP_A_UNAVAIL;

  mutt_message _("Logging in...");

  snprintf (buf, sizeof (buf), "USER %s\r\n", pop_data->conn->account.user);
  ret = pop_query (pop_data, buf, sizeof (buf));

  if (pop_data->cmd_user == CMD_UNKNOWN) {
    if (ret == PQ_OK) {
      pop_data->cmd_user = CMD_AVAILABLE;

      debug_print (1, ("set USER capability\n"));
    }

    if (ret == PQ_ERR) {
      pop_data->cmd_user = CMD_NOT_AVAILABLE;

      debug_print (1, ("unset USER capability\n"));
      snprintf (pop_data->err_msg, sizeof (pop_data->err_msg),
                _("Command USER is not supported by server."));
    }
  }

  if (ret == PQ_OK) {
    snprintf (buf, sizeof (buf), "PASS %s\r\n", pop_data->conn->account.pass);
    ret = pop_query_d (pop_data, buf, sizeof (buf),
#ifdef DEBUG
    /* don't print the password unless we're at the ungodly debugging level */
    DebugLevel < M_SOCK_LOG_FULL ? "PASS *\r\n" :
#endif
    NULL);
  }

  switch (ret) {
  case PQ_OK:
    return POP_A_SUCCESS;
  case PQ_NOT_CONNECTED:
    return POP_A_SOCKET;
  }

  mutt_error ("%s %s", _("Login failed."), pop_data->err_msg);
  mutt_sleep (2);

  return POP_A_FAILURE;
}

static pop_auth_t pop_authenticators[] = {
#ifdef USE_SASL
  {pop_auth_sasl, NULL},
#endif
  {pop_auth_apop, "apop"},
  {pop_auth_user, "user"},
  {NULL}
};

/*
 * Authentication
 *  0 - successful,
 * -1 - conection lost,
 * -2 - login failed,
 * -3 - authentication canceled.
*/
pop_query_status pop_authenticate (POP_DATA * pop_data)
{
  ACCOUNT *acct = &pop_data->conn->account;
  pop_auth_t *authenticator;
  char *methods;
  char *comma;
  char *method;
  int attempts = 0;
  int ret = POP_A_UNAVAIL;

  if (mutt_account_getuser (acct) || !acct->user[0] ||
      mutt_account_getpass (acct) || !acct->pass[0])
    return PFD_FUNCT_ERROR;

  if (PopAuthenticators && *PopAuthenticators) {
    /* Try user-specified list of authentication methods */
    methods = str_dup (PopAuthenticators);
    method = methods;

    while (method) {
      comma = strchr (method, ':');
      if (comma)
        *comma++ = '\0';
      debug_print (2, ("Trying method %s\n", method));
      authenticator = pop_authenticators;

      while (authenticator->authenticate) {
        if (!authenticator->method ||
            !ascii_strcasecmp (authenticator->method, method)) {
          ret = authenticator->authenticate (pop_data, method);
          if (ret == POP_A_SOCKET)
            switch (pop_connect (pop_data)) {
            case PQ_OK:
              {
                ret = authenticator->authenticate (pop_data, method);
                break;
              }
            case PQ_ERR:
              ret = POP_A_FAILURE;
            }

          if (ret != POP_A_UNAVAIL)
            attempts++;
          if (ret == POP_A_SUCCESS || ret == POP_A_SOCKET ||
              (ret == POP_A_FAILURE && !option (OPTPOPAUTHTRYALL))) {
            comma = NULL;
            break;
          }
        }
        authenticator++;
      }

      method = comma;
    }

    FREE (&methods);
  }
  else {
    /* Fall back to default: any authenticator */
    debug_print (2, ("Using any available method.\n"));
    authenticator = pop_authenticators;

    while (authenticator->authenticate) {
      ret = authenticator->authenticate (pop_data, authenticator->method);
      if (ret == POP_A_SOCKET)
        switch (pop_connect (pop_data)) {
        case PQ_OK:
          {
            ret =
              authenticator->authenticate (pop_data, authenticator->method);
            break;
          }
        case PQ_ERR:
          ret = POP_A_FAILURE;
        }

      if (ret != POP_A_UNAVAIL)
        attempts++;
      if (ret == POP_A_SUCCESS || ret == POP_A_SOCKET ||
          (ret == POP_A_FAILURE && !option (OPTPOPAUTHTRYALL)))
        break;

      authenticator++;
    }
  }

  switch (ret) {
  case POP_A_SUCCESS:
    return PQ_OK;
  case POP_A_SOCKET:
    return PQ_NOT_CONNECTED;
  case POP_A_UNAVAIL:
    if (!attempts)
      mutt_error (_("No authenticators available"));
  }

  return PQ_ERR;
}
