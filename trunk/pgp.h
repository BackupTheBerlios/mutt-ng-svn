/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996,1997 Michael R. Elkins <me@mutt.org>
 * Copyright (C) 1999-2000 Thomas Roessler <roessler@does-not-exist.org>
 * Copyright (C) 2004 g10 Code GmbH
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#ifdef CRYPT_BACKEND_CLASSIC_PGP

#include "mutt_crypt.h"
#include "pgplib.h"


/* prototypes */

int pgp_use_gpg_agent (void);

int pgp_check_traditional (FILE *, BODY *, int);
BODY *pgp_decrypt_part (BODY *, STATE *, FILE *, BODY *);
BODY *pgp_make_key_attachment (char *);
const char *pgp_micalg (const char *fname);

char *_pgp_keyid (pgp_key_t);
char *pgp_keyid (pgp_key_t);


int mutt_check_pgp (HEADER * h);

int pgp_decrypt_mime (FILE *, FILE **, BODY *, BODY **);

/* int pgp_string_matches_hint (const char *s, LIST * hints); */

/* pgp_key_t gpg_get_candidates (struct pgp_vinfo *, pgp_ring_t, LIST *); */
pgp_key_t pgp_ask_for_key (char *, char *, short, pgp_ring_t);
pgp_key_t pgp_get_candidates (pgp_ring_t, LIST *);
pgp_key_t pgp_getkeybyaddr (ADDRESS *, short, pgp_ring_t);
pgp_key_t pgp_getkeybystr (char *, short, pgp_ring_t);

char *pgp_findKeys (ADDRESS * to, ADDRESS * cc, ADDRESS * bcc);

void pgp_forget_passphrase (void);
int pgp_application_pgp_handler (BODY *, STATE *);
int pgp_encrypted_handler (BODY *, STATE *);
void pgp_extract_keys_from_attachment_list (FILE * fp, int tag, BODY * top);
void pgp_void_passphrase (void);
int pgp_valid_passphrase (void);


/* The PGP invocation interface - not really beautiful. */

pid_t pgp_invoke_decode (FILE ** pgpin, FILE ** pgpout, FILE ** pgperr,
                         int pgpinfd, int pgpoutfd, int pgperrfd,
                         const char *fname, short need_passphrase);
pid_t pgp_invoke_verify (FILE ** pgpin, FILE ** pgpout, FILE ** pgperr,
                         int pgpinfd, int pgpoutfd, int pgperrfd,
                         const char *fname, const char *sig_fname);
pid_t pgp_invoke_decrypt (FILE ** pgpin, FILE ** pgpout, FILE ** pgperr,
                          int pgpinfd, int pgpoutfd, int pgperrfd,
                          const char *fname);
pid_t pgp_invoke_sign (FILE ** pgpin, FILE ** pgpout, FILE ** pgperr,
                       int pgpinfd, int pgpoutfd, int pgperrfd,
                       const char *fname);
pid_t pgp_invoke_encrypt (FILE ** pgpin, FILE ** pgpout, FILE ** pgperr,
                          int pgpinfd, int pgpoutfd, int pgperrfd,
                          const char *fname, const char *uids, int sign);
pid_t pgp_invoke_export (FILE ** pgpin, FILE ** pgpout, FILE ** pgperr,
                         int pgpinfd, int pgpoutfd, int pgperrfd,
                         const char *uids);
pid_t pgp_invoke_verify_key (FILE ** pgpin, FILE ** pgpout, FILE ** pgperr,
                             int pgpinfd, int pgpoutfd, int pgperrfd,
                             const char *uids);
pid_t pgp_invoke_list_keys (FILE ** pgpin, FILE ** pgpout, FILE ** pgperr,
                            int pgpinfd, int pgpoutfd, int pgperrfd,
                            pgp_ring_t keyring, LIST * hints);
pid_t pgp_invoke_traditional (FILE ** pgpin, FILE ** pgpout, FILE ** pgperr,
                              int pgpinfd, int pgpoutfd, int pgperrfd,
                              const char *fname, const char *uids, int flags);


void pgp_invoke_import (const char *fname);
void pgp_invoke_getkeys (ADDRESS *);


/* private ? */
int pgp_verify_one (BODY *, STATE *, const char *);
BODY *pgp_traditional_encryptsign (BODY *, int, char *);
BODY *pgp_encrypt_message (BODY *, char *, int);
BODY *pgp_sign_message (BODY *);

int pgp_send_menu (HEADER * msg, int *redraw);

#endif /* CRYPT_BACKEND_CLASSIC_PGP */
