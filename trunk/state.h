/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */
#ifndef _MUTT_STATE_H
#define _MUTT_STATE_H

#include "charset.h"

typedef struct {
  FILE *fpin;
  FILE *fpout;
  char *prefix;
  int flags;
} STATE;

#define BUFI_SIZE 1000
#define BUFO_SIZE 2000

/* flags for the STATE struct */
#define M_DISPLAY	(1<<0)  /* output is displayed to the user */
#define M_VERIFY	(1<<1)  /* perform signature verification */
#define M_PENDINGPREFIX (1<<2)  /* prefix to write, but character must follow */
#define M_WEED          (1<<3)  /* weed headers even when not in display mode */
#define M_CHARCONV	(1<<4)  /* Do character set conversions */
#define M_PRINTING	(1<<5)  /* are we printing? - M_DISPLAY "light" */
#define M_REPLYING	(1<<6)  /* are we replying? */
#define M_FIRSTDONE	(1<<7)  /* the first attachment has been done */

#define state_set_prefix(s) ((s)->flags |= M_PENDINGPREFIX)
#define state_reset_prefix(s) ((s)->flags &= ~M_PENDINGPREFIX)
#define state_puts(x,y) fputs(x,(y)->fpout)
#define state_putc(x,y) fputc(x,(y)->fpout)

void state_mark_attach (STATE *);
void state_attach_puts (const char *, STATE *);
void state_prefix_putc (char, STATE *);
int state_printf (STATE *, const char *, ...);

void mutt_convert_to_state (iconv_t, char*, size_t*, STATE*);

#endif /* !_MUTT_STATE_H */
