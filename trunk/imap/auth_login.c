/*
 * Copyright notice from original mutt:
 * Copyright (C) 1999-2001 Brendan Cully <brendan@kublai.com>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

/* plain LOGIN support */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "lib/intl.h"
#include "lib/debug.h"

#include "mutt.h"
#include "imap_private.h"
#include "auth.h"

/* imap_auth_login: Plain LOGIN support */
imap_auth_res_t imap_auth_login (IMAP_DATA * idata, const char *method)
{
  char q_user[SHORT_STRING], q_pass[SHORT_STRING];
  char buf[STRING];
  int rc;

  if (mutt_bit_isset (idata->capabilities, LOGINDISABLED)) {
    mutt_message _("LOGIN disabled on this server.");

    return IMAP_AUTH_UNAVAIL;
  }

  if (mutt_account_getlogin (&idata->conn->account))
    return IMAP_AUTH_FAILURE;
  if (mutt_account_getpass (&idata->conn->account))
    return IMAP_AUTH_FAILURE;

  mutt_message _("Logging in...");

  imap_quote_string (q_user, sizeof (q_user), idata->conn->account.login);
  imap_quote_string (q_pass, sizeof (q_pass), idata->conn->account.pass);

#ifdef DEBUG
  /* don't print the password unless we're at the ungodly debugging level
   * of 5 or higher */

  if (DebugLevel < IMAP_LOG_PASS)
    debug_print (2, ("Sending LOGIN command for %s...\n",
                idata->conn->account.user));
#endif

  snprintf (buf, sizeof (buf), "LOGIN %s %s", q_user, q_pass);
  rc = imap_exec (idata, buf, IMAP_CMD_FAIL_OK | IMAP_CMD_PASS);

  if (!rc)
    return IMAP_AUTH_SUCCESS;

  mutt_error _("Login failed.");

  mutt_sleep (2);
  return IMAP_AUTH_FAILURE;
}
