/*
 * Copyright notice from original mutt:
 * Copyright (C) 2000-3 Brendan Cully <brendan@kublai.com>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

/* SASL login/authentication code */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "mutt.h"
#include "mutt_sasl.h"
#include "imap_private.h"
#include "auth.h"

#include "lib/mem.h"
#include "lib/intl.h"
#include "lib/debug.h"

#ifdef USE_SASL2
#include <sasl/sasl.h>
#include <sasl/saslutil.h>
#else
#include <sasl.h>
#include <saslutil.h>
#endif

/* imap_auth_sasl: Default authenticator if available. */
imap_auth_res_t imap_auth_sasl (IMAP_DATA * idata, const char *method)
{
  sasl_conn_t *saslconn;
  sasl_interact_t *interaction = NULL;
  int rc, irc;
  char buf[HUGE_STRING];
  const char *mech;

#ifdef USE_SASL2
  const char *pc = NULL;
#else
  char *pc = NULL;
#endif
  unsigned int len, olen;
  unsigned char client_start;

  if (mutt_sasl_client_new (idata->conn, &saslconn) < 0) {
    debug_print (1, ("Error allocating SASL connection.\n"));
    return IMAP_AUTH_FAILURE;
  }

  rc = SASL_FAIL;

  /* If the user hasn't specified a method, use any available */
  if (!method) {
    method = idata->capstr;

    /* hack for SASL ANONYMOUS support:
     * 1. Fetch username. If it's "" or "anonymous" then
     * 2. attempt sasl_client_start with only "AUTH=ANONYMOUS" capability
     * 3. if sasl_client_start fails, fall through... */

    if (mutt_account_getuser (&idata->conn->account))
      return IMAP_AUTH_FAILURE;

    if (mutt_bit_isset (idata->capabilities, AUTH_ANON) &&
        (!idata->conn->account.user[0] ||
         !ascii_strncmp (idata->conn->account.user, "anonymous", 9)))
#ifdef USE_SASL2
      rc = sasl_client_start (saslconn, "AUTH=ANONYMOUS", NULL, &pc, &olen,
                              &mech);
#else
      rc =
        sasl_client_start (saslconn, "AUTH=ANONYMOUS", NULL, NULL, &pc, &olen,
                           &mech);
#endif
  }

  if (rc != SASL_OK && rc != SASL_CONTINUE)
    do {
#ifdef USE_SASL2
      rc = sasl_client_start (saslconn, method, &interaction,
                              &pc, &olen, &mech);
#else
      rc = sasl_client_start (saslconn, method, NULL, &interaction,
                              &pc, &olen, &mech);
#endif
      if (rc == SASL_INTERACT)
        mutt_sasl_interact (interaction);
    }
    while (rc == SASL_INTERACT);

  client_start = (olen > 0);

  if (rc != SASL_OK && rc != SASL_CONTINUE) {
    if (method)
      debug_print (2, ("%s unavailable\n", method));
    else
      debug_print (1, ("Failure starting authentication exchange. No shared mechanisms?\n"));
    /* SASL doesn't support LOGIN, so fall back */

    return IMAP_AUTH_UNAVAIL;
  }

  mutt_message (_("Authenticating (%s)..."), mech);

  snprintf (buf, sizeof (buf), "AUTHENTICATE %s", mech);
  imap_cmd_start (idata, buf);
  irc = IMAP_CMD_CONTINUE;

  /* looping protocol */
  while (rc == SASL_CONTINUE || olen > 0) {
    do
      irc = imap_cmd_step (idata);
    while (irc == IMAP_CMD_CONTINUE);

    if (method && irc == IMAP_CMD_NO) {
      debug_print (2, ("%s failed\n", method));
      sasl_dispose (&saslconn);
      return IMAP_AUTH_UNAVAIL;
    }

    if (irc == IMAP_CMD_BAD || irc == IMAP_CMD_NO)
      goto bail;

    if (irc == IMAP_CMD_RESPOND) {
#ifdef USE_SASL2
      if (sasl_decode64
          (idata->cmd.buf + 2, mutt_strlen (idata->cmd.buf + 2), buf,
           LONG_STRING - 1,
#else
      if (sasl_decode64 (idata->cmd.buf + 2, mutt_strlen (idata->cmd.buf + 2), buf,
#endif
                         &len) != SASL_OK) {
        debug_print (1, ("error base64-decoding server response.\n"));
        goto bail;
      }
    }

    if (!client_start) {
      do {
        rc = sasl_client_step (saslconn, buf, len, &interaction, &pc, &olen);
        if (rc == SASL_INTERACT)
          mutt_sasl_interact (interaction);
      }
      while (rc == SASL_INTERACT);
    }
    else
      client_start = 0;

    /* send out response, or line break if none needed */
    if (olen) {
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

    if (irc == IMAP_CMD_RESPOND) {
      strfcpy (buf + olen, "\r\n", sizeof (buf) - olen);
      mutt_socket_write (idata->conn, buf);
    }

    /* If SASL has errored out, send an abort string to the server */
    if (rc < 0) {
      mutt_socket_write (idata->conn, "*\r\n");
      debug_print (1, ("sasl_client_step error %d\n", rc));
    }

    olen = 0;
  }

  while (irc != IMAP_CMD_OK)
    if ((irc = imap_cmd_step (idata)) != IMAP_CMD_CONTINUE)
      break;

  if (rc != SASL_OK)
    goto bail;

  if (imap_code (idata->cmd.buf)) {
    mutt_sasl_setup_conn (idata->conn, saslconn);
    return IMAP_AUTH_SUCCESS;
  }

bail:
  mutt_error _("SASL authentication failed.");
  mutt_sleep (2);
  sasl_dispose (&saslconn);

  return IMAP_AUTH_FAILURE;
}
