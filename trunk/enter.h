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

/* flags for mutt_enter_string() */
#define  M_ALIAS   1            /* do alias "completion" by calling up the alias-menu */
#define  M_FILE    (1<<1)       /* do file completion */
#define  M_EFILE   (1<<2)       /* do file completion, plus incoming folders */
#define  M_CMD     (1<<3)       /* do completion on previous word */
#define  M_PASS    (1<<4)       /* password mode (no echo) */
#define  M_CLEAR   (1<<5)       /* clear input if printable character is pressed */
#define  M_COMMAND (1<<6)       /* do command completion */
#define  M_PATTERN (1<<7)       /* pattern mode - only used for history classes */
#define  M_LASTFOLDER (1<<8)    /* last-folder mode - hack hack hack */

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
