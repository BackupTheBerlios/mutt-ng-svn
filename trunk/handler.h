/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */
#ifndef _MUTT_HANDLER_H
#define _MUTT_HANDLER_H

#include "state.h"

void mutt_body_handler (BODY *, STATE *);
void mutt_decode_attachment (BODY *, STATE *);
void mutt_decode_base64 (STATE * s, long len, int istext, iconv_t cd);

#endif /* !_MUTT_HANDLER_H */
