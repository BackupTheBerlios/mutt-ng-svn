/*
 * Copyright notice from original mutt:
 * Copyright (C) 1999-2000 Thomas Roessler <roessler@does-not-exist.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

/*
 * Mixmaster support for Mutt
 */

#ifndef _REMAILER_H
#define _REMAILER_H

#ifdef MIXMASTER

#define MIX_CAP_COMPRESS  (1 << 0)
#define MIX_CAP_MIDDLEMAN (1 << 1)
#define MIX_CAP_NEWSPOST  (1 << 2)
#define MIX_CAP_NEWSMAIL  (1 << 3)

/* Mixmaster's maximum chain length.  Don't change this. */

#define MAXMIXES 19

struct type2 {
  int num;
  char *shortname;
  char *addr;
  char *ver;
  int caps;
};

typedef struct type2 REMAILER;


struct mixchain {
  size_t cl;
  int ch[MAXMIXES];
};

typedef struct mixchain MIXCHAIN;

int mix_send_message (LIST *, const char *);
int mix_check_message (HEADER * msg);
void mix_make_chain (LIST **, int *);

#endif /* MIXMASTER */

#endif /* _REMAILER_H */
