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

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include "ascii.h"

int ascii_isupper (int c)
{
  return (c >= 'A') && (c <= 'Z');
}

int ascii_islower (int c)
{
  return (c >= 'a') && (c <= 'z');
}

int ascii_toupper (int c)
{
  if (ascii_islower (c))
    return c & ~32;

  return c;
}

int ascii_tolower (int c)
{
  if (ascii_isupper (c))
    return c | 32;

  return c;
}

int ascii_strcasecmp (const char *a, const char *b)
{
  int i;

  if (a == b)
    return 0;
  if (a == NULL && b)
    return -1;
  if (b == NULL && a)
    return 1;

  for (; *a || *b; a++, b++) {
    if ((i = ascii_tolower (*a) - ascii_tolower (*b)))
      return i;
  }

  return 0;
}

int ascii_strncasecmp (const char *a, const char *b, int n)
{
  int i, j;

  if (a == b)
    return 0;
  if (a == NULL && b)
    return -1;
  if (b == NULL && a)
    return 1;

  for (j = 0; (*a || *b) && j < n; a++, b++, j++) {
    if ((i = ascii_tolower (*a) - ascii_tolower (*b)))
      return i;
  }

  return 0;
}
