/*
 * Copyright notice from original mutt:
 * Copyright (C) 1999-2000 Brendan Cully <brendan@kublai.com>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

/* IMAP login/authentication code */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "mutt.h"
#include "imap_private.h"
#include "auth.h"

#include "lib/intl.h"

/* this is basically a stripped-down version of the cram-md5 method. */
imap_auth_res_t imap_auth_anon (IMAP_DATA * idata, const char *method)
{
  int rc;

  if (!mutt_bit_isset (idata->capabilities, AUTH_ANON))
    return IMAP_AUTH_UNAVAIL;

  if (mutt_account_getuser (&idata->conn->account))
    return IMAP_AUTH_FAILURE;

  if (idata->conn->account.user[0] != '\0')
    return IMAP_AUTH_UNAVAIL;

  mutt_message _("Authenticating (anonymous)...");

  imap_cmd_start (idata, "AUTHENTICATE ANONYMOUS");

  do
    rc = imap_cmd_step (idata);
  while (rc == IMAP_CMD_CONTINUE);

  if (rc != IMAP_CMD_RESPOND) {
    dprint (1, (debugfile, "Invalid response from server.\n"));
    goto bail;
  }

  mutt_socket_write (idata->conn, "ZHVtbXkK\r\n");      /* base64 ("dummy") */

  do
    rc = imap_cmd_step (idata);
  while (rc == IMAP_CMD_CONTINUE);

  if (rc != IMAP_CMD_OK) {
    dprint (1, (debugfile, "Error receiving server response.\n"));
    goto bail;
  }

  if (imap_code (idata->cmd.buf))
    return IMAP_AUTH_SUCCESS;

bail:
  mutt_error _("Anonymous authentication failed.");
  mutt_sleep (2);
  return IMAP_AUTH_FAILURE;
}
