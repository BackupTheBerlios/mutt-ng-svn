/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */
#ifndef _MUTT_BUFFER_H
#define _MUTT_BUFFER_H

#include "mutt.h"

int mutt_extract_token (BUFFER *, BUFFER *, int);
BUFFER *mutt_buffer_init (BUFFER *);
BUFFER *mutt_buffer_from (BUFFER *, char *);
void mutt_buffer_free (BUFFER **);
void mutt_buffer_add (BUFFER *, const char *, size_t);
void mutt_buffer_addstr (BUFFER *, const char *);
void mutt_buffer_addch (BUFFER *, char);

#endif /* !_MUTT_BUFFER_H */
