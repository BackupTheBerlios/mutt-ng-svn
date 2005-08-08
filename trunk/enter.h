/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */
#ifndef _MUTT_ENTER_H
#define _MUTT_ENTER_H

typedef struct {
  wchar_t *wbuf;
  size_t wbuflen;
  size_t lastchar;
  size_t curpos;
  size_t begin;
  int tabs;
} ENTER_STATE;

void mutt_free_enter_state (ENTER_STATE **);
int mutt_enter_string (char *buf, size_t buflen, int y, int x, int flags);
int _mutt_enter_string (char *, size_t, int, int, int, int, char ***, int *,
                        ENTER_STATE *);

#endif /* !_MUTT_ENTER_H */
