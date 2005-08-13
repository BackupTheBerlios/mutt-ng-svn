/*
 * Copyright notice from original mutt:
 * Copyright (C) 2004 g10 Code GmbH
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

/* 
    This is a crytpo module wrapping the classic smime code.
 */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "crypt-mod.h"
#include "smime.h"

static void crypt_mod_smime_void_passphrase (void)
{
  smime_void_passphrase ();
}

static int crypt_mod_smime_valid_passphrase (void)
{
  return smime_valid_passphrase ();
}

static int crypt_mod_smime_decrypt_mime (FILE * a, FILE ** b, BODY * c,
                                         BODY ** d)
{
  return smime_decrypt_mime (a, b, c, d);
}
static int crypt_mod_smime_application_handler (BODY * m, STATE * s)
{
  return smime_application_smime_handler (m, s);
}

static char *crypt_mod_smime_findkeys (ADDRESS * to, ADDRESS * cc,
                                       ADDRESS * bcc)
{
  return smime_findKeys (to, cc, bcc);
}

static BODY *crypt_mod_smime_sign_message (BODY * a)
{
  return smime_sign_message (a);
}

static int crypt_mod_smime_verify_one (BODY * sigbdy, STATE * s,
                                       const char *tempf)
{
  return smime_verify_one (sigbdy, s, tempf);
}

static int crypt_mod_smime_send_menu (HEADER * msg, int *redraw)
{
  return smime_send_menu (msg, redraw);
}

static void crypt_mod_smime_getkeys (ENVELOPE * env)
{
  smime_getkeys (env);
}

static int crypt_mod_smime_verify_sender (HEADER * h)
{
  return smime_verify_sender (h);
}

static BODY *crypt_mod_smime_build_smime_entity (BODY * a, char *certlist)
{
  return smime_build_smime_entity (a, certlist);
}

static void crypt_mod_smime_invoke_import (char *infile, char *mailbox)
{
  smime_invoke_import (infile, mailbox);
}


struct crypt_module_specs crypt_mod_smime_classic = { APPLICATION_SMIME,
  {
   NULL,                        /* init */
   crypt_mod_smime_void_passphrase,
   crypt_mod_smime_valid_passphrase,
   crypt_mod_smime_decrypt_mime,
   crypt_mod_smime_application_handler,
   NULL,                        /* encrypted_handler */
   crypt_mod_smime_findkeys,
   crypt_mod_smime_sign_message,
   crypt_mod_smime_verify_one,
   crypt_mod_smime_send_menu,

   NULL,                        /* pgp_encrypt_message */
   NULL,                        /* pgp_make_key_attachment */
   NULL,                        /* pgp_check_traditional */
   NULL,                        /* pgp_traditional_encryptsign */
   NULL,                        /* pgp_invoke_getkeys */
   NULL,                        /* pgp_invoke_import */
   NULL,                        /* pgp_extract_keys_from_attachment_list */

   crypt_mod_smime_getkeys,
   crypt_mod_smime_verify_sender,
   crypt_mod_smime_build_smime_entity,
   crypt_mod_smime_invoke_import,
   }
};
