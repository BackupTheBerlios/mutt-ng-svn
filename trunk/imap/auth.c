/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-8 Michael R. Elkins <me@mutt.org>
 * Copyright (C) 1996-9 Brandon Long <blong@fiction.net>
 * Copyright (C) 1999-2001 Brendan Cully <brendan@kublai.com>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

/* IMAP login/authentication code */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "lib/mem.h"
#include "lib/intl.h"
#include "lib/debug.h"

#include "mutt.h"
#include "imap_private.h"
#include "auth.h"

static imap_auth_t imap_authenticators[] = {
#ifdef USE_SASL
  {imap_auth_sasl, NULL},
#else
  {imap_auth_anon, "anonymous"},
#endif
#ifdef USE_GSS
  {imap_auth_gss, "gssapi"},
#endif
  /* SASL includes CRAM-MD5 (and GSSAPI, but that's not enabled by default) */
#ifndef USE_SASL
  {imap_auth_cram_md5, "cram-md5"},
#endif
  {imap_auth_login, "login"},

  {NULL}
};

/* imap_authenticate: Attempt to authenticate using either user-specified
 *   authentication method if specified, or any. */
int imap_authenticate (IMAP_DATA * idata)
{
  imap_auth_t *authenticator;
  char *methods;
  char *method;
  char *delim;
  int r = -1;

  if (ImapAuthenticators && *ImapAuthenticators) {
    /* Try user-specified list of authentication methods */
    methods = safe_strdup (ImapAuthenticators);

    for (method = methods; method; method = delim) {
      delim = strchr (method, ':');
      if (delim)
        *delim++ = '\0';
      if (!method[0])
        continue;

      debug_print (2, ("Trying method %s\n", method));
      authenticator = imap_authenticators;

      while (authenticator->authenticate) {
        if (!authenticator->method ||
            !ascii_strcasecmp (authenticator->method, method))
          if ((r = authenticator->authenticate (idata, method)) !=
              IMAP_AUTH_UNAVAIL) {
            FREE (&methods);
            return r;
          }

        authenticator++;
      }
    }

    FREE (&methods);
  }
  else {
    /* Fall back to default: any authenticator */
    debug_print (2, ("Using any available method.\n"));
    authenticator = imap_authenticators;

    while (authenticator->authenticate) {
      if ((r =
           authenticator->authenticate (idata, NULL)) != IMAP_AUTH_UNAVAIL)
        return r;
      authenticator++;
    }
  }

  if (r == IMAP_AUTH_UNAVAIL) {
    mutt_error (_("No authenticators available"));
    mutt_sleep (1);
  }

  return r;
}
