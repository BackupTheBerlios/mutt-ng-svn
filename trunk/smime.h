/*
 * Copyright notice from original mutt:
 * Copyright (C) 2001,2002 Oliver Ehli <elmy@acm.org>
 * Copyright (C) 2004 g10 Code GmbH
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#ifdef CRYPT_BACKEND_CLASSIC_SMIME

#include "mutt_crypt.h"





void smime_void_passphrase (void);
int smime_valid_passphrase (void);

int smime_decrypt_mime (FILE *, FILE **, BODY *, BODY **);

void smime_application_smime_handler (BODY *, STATE *);


BODY *smime_sign_message (BODY *);

BODY *smime_build_smime_entity (BODY *, char *);

int smime_verify_one (BODY *, STATE *, const char *);


int smime_verify_sender (HEADER *);


char *smime_get_field_from_db (char *, char *, short, short);

void smime_getkeys (ENVELOPE *);

char *smime_ask_for_key (char *, char *, short);

char *smime_findKeys (ADDRESS * to, ADDRESS * cc, ADDRESS * bcc);

void smime_invoke_import (char *, char *);

int smime_send_menu (HEADER * msg, int *redraw);

#endif
