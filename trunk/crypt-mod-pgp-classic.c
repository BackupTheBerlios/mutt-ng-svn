/*
 * Copyright notice from original mutt:
 * Copyright (C) 2004 g10 Code GmbH
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

/* 
    This is a crytpo module wrapping the classic pgp code.
 */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "crypt-mod.h"
#include "pgp.h"

static void crypt_mod_pgp_void_passphrase (void)
{
  pgp_void_passphrase ();
}

static int crypt_mod_pgp_valid_passphrase (void)
{
  return pgp_valid_passphrase ();
}

static int crypt_mod_pgp_decrypt_mime (FILE * a, FILE ** b, BODY * c,
                                       BODY ** d)
{
  return pgp_decrypt_mime (a, b, c, d);
}
static int crypt_mod_pgp_application_handler (BODY * m, STATE * s)
{
  return pgp_application_pgp_handler (m, s);
}

static char *crypt_mod_pgp_findkeys (ADDRESS * to, ADDRESS * cc,
                                     ADDRESS * bcc)
{
  return pgp_findKeys (to, cc, bcc);
}

static BODY *crypt_mod_pgp_sign_message (BODY * a)
{
  return pgp_sign_message (a);
}

static int crypt_mod_pgp_verify_one (BODY * sigbdy, STATE * s,
                                     const char *tempf)
{
  return pgp_verify_one (sigbdy, s, tempf);
}

static int crypt_mod_pgp_send_menu (HEADER * msg, int *redraw)
{
  return pgp_send_menu (msg, redraw);
}

static BODY *crypt_mod_pgp_encrypt_message (BODY * a, char *keylist, int sign)
{
  return pgp_encrypt_message (a, keylist, sign);
}

static BODY *crypt_mod_pgp_make_key_attachment (char *tempf)
{
  return pgp_make_key_attachment (tempf);
}

static int crypt_mod_pgp_check_traditional (FILE * fp, BODY * b,
                                            int tagged_only)
{
  return pgp_check_traditional (fp, b, tagged_only);
}

static BODY *crypt_mod_pgp_traditional_encryptsign (BODY * a, int flags,
                                                    char *keylist)
{
  return pgp_traditional_encryptsign (a, flags, keylist);
}

static int crypt_mod_pgp_encrypted_handler (BODY * m, STATE * s)
{
  return pgp_encrypted_handler (m, s);
}

static void crypt_mod_pgp_invoke_getkeys (ADDRESS * addr)
{
  pgp_invoke_getkeys (addr);
}

static void crypt_mod_pgp_invoke_import (const char *fname)
{
  pgp_invoke_import (fname);
}

static void crypt_mod_pgp_extract_keys_from_attachment_list (FILE * fp,
                                                             int tag,
                                                             BODY * top)
{
  pgp_extract_keys_from_attachment_list (fp, tag, top);
}

struct crypt_module_specs crypt_mod_pgp_classic = { APPLICATION_PGP,
  {
   NULL,                        /* init */
   crypt_mod_pgp_void_passphrase,
   crypt_mod_pgp_valid_passphrase,
   crypt_mod_pgp_decrypt_mime,
   crypt_mod_pgp_application_handler,
   crypt_mod_pgp_encrypted_handler,
   crypt_mod_pgp_findkeys,
   crypt_mod_pgp_sign_message,
   crypt_mod_pgp_verify_one,
   crypt_mod_pgp_send_menu,

   crypt_mod_pgp_encrypt_message,
   crypt_mod_pgp_make_key_attachment,
   crypt_mod_pgp_check_traditional,
   crypt_mod_pgp_traditional_encryptsign,
   crypt_mod_pgp_invoke_getkeys,
   crypt_mod_pgp_invoke_import,
   crypt_mod_pgp_extract_keys_from_attachment_list,

   NULL,                        /* smime_getkeys */
   NULL,                        /* smime_verify_sender */
   NULL,                        /* smime_build_smime_entity */
   NULL,                        /* smime_invoke_import */
   }
};
