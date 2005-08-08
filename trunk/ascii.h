/*
 * Copyright notice from original mutt:
 * Copyright (C) 2001 Thomas Roessler <roessler@does-not-exist.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

/* 
 * Versions of the string comparison functions which are
 * locale-insensitive.
 */

#ifndef _ASCII_H
# define _ASCII_H

int ascii_isupper (int c);
int ascii_islower (int c);
int ascii_toupper (int c);
int ascii_tolower (int c);
int ascii_strcasecmp (const char *a, const char *b);
int ascii_strncasecmp (const char *a, const char *b, int n);

#define ascii_strcmp(a,b) str_cmp(a,b)
#define ascii_strncmp(a,b,c) str_ncmp(a,b,c)

#endif
