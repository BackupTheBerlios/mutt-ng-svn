/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#ifndef MAPPING_H
#define MAPPING_H

struct mapping_t {
  char *name;
  int value;
};

char *mutt_getnamebyvalue (int, const struct mapping_t *);
char *mutt_compile_help (char *, size_t, int, struct mapping_t *);

int mutt_getvaluebyname (const char *, const struct mapping_t *);

#endif
