/*
 * Copyright notice from original mutt:
 * Copyright (C) 2004 g10 Code GmbH
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#ifndef CRYPT_GPGME_H
#define CRYPT_GPGME_H

#include "mutt_crypt.h"

void pgp_gpgme_init (void);
void smime_gpgme_init (void);

char *pgp_gpgme_findkeys (ADDRESS * to, ADDRESS * cc, ADDRESS * bcc);
char *smime_gpgme_findkeys (ADDRESS * to, ADDRESS * cc, ADDRESS * bcc);

BODY *pgp_gpgme_encrypt_message (BODY * a, char *keylist, int sign);
BODY *smime_gpgme_build_smime_entity (BODY * a, char *keylist);

int pgp_gpgme_decrypt_mime (FILE * fpin, FILE ** fpout, BODY * b,
                            BODY ** cur);
int smime_gpgme_decrypt_mime (FILE * fpin, FILE ** fpout, BODY * b,
                              BODY ** cur);

int pgp_gpgme_check_traditional (FILE * fp, BODY * b, int tagged_only);

void pgp_gpgme_application_handler (BODY * m, STATE * s);
void smime_gpgme_application_handler (BODY * a, STATE * s);
void pgp_gpgme_encrypted_handler (BODY * a, STATE * s);

BODY *pgp_gpgme_make_key_attachment (char *tempf);

BODY *pgp_gpgme_sign_message (BODY * a);
BODY *smime_gpgme_sign_message (BODY * a);

int pgp_gpgme_verify_one (BODY * sigbdy, STATE * s, const char *tempfile);
int smime_gpgme_verify_one (BODY * sigbdy, STATE * s, const char *tempfile);

int pgp_gpgme_send_menu (HEADER * msg, int *redraw);
int smime_gpgme_send_menu (HEADER * msg, int *redraw);

#endif
