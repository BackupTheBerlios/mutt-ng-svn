/*
 * Copyright notice from original mutt:
 * Copyright (C) 2000-1 Brendan Cully <brendan@kublai.com>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

/* common defs for authenticators. A good place to set up a generic callback
 * system */

#ifndef _IMAP_AUTH_H
#define _IMAP_AUTH_H 1

typedef enum {
  IMAP_AUTH_SUCCESS = 0,
  IMAP_AUTH_FAILURE,
  IMAP_AUTH_UNAVAIL
} imap_auth_res_t;


typedef struct {
  /* do authentication, using named method or any available if method is NULL */
  imap_auth_res_t (*authenticate) (IMAP_DATA * idata, const char *method);
  /* name of authentication method supported, NULL means variable. If this
   * is not null, authenticate may ignore the second parameter. */
  const char *method;
} imap_auth_t;

/* external authenticator prototypes */
#ifndef USE_SASL
imap_auth_res_t imap_auth_anon (IMAP_DATA * idata, const char *method);
imap_auth_res_t imap_auth_cram_md5 (IMAP_DATA * idata, const char *method);
#endif
imap_auth_res_t imap_auth_login (IMAP_DATA * idata, const char *method);

#ifdef USE_GSS
imap_auth_res_t imap_auth_gss (IMAP_DATA * idata, const char *method);
#endif
#ifdef USE_SASL
imap_auth_res_t imap_auth_sasl (IMAP_DATA * idata, const char *method);
#endif

#endif /* _IMAP_AUTH_H */
