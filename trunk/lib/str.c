/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 * Copyright (C) 1999-2000 Thomas Roessler <roessler@does-not-exist.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "str.h"

#include "mem.h"

char *safe_strdup (const char *s)
{
  char *p;
  size_t l;

  if (!s || !*s)
    return 0;
  l = mutt_strlen (s) + 1;
  p = (char *) safe_malloc (l);
  memcpy (p, s, l);
  return (p);
}

char *safe_strcat (char *d, size_t l, const char *s)
{
  char *p = d;

  if (!l)
    return d;

  l--;                          /* Space for the trailing '\0'. */

  for (; *d && l; l--)
    d++;
  for (; *s && l; l--)
    *d++ = *s++;

  *d = '\0';

  return p;
}

char *safe_strncat (char *d, size_t l, const char *s, size_t sl)
{
  char *p = d;

  if (!l)
    return d;

  l--;                          /* Space for the trailing '\0'. */

  for (; *d && l; l--)
    d++;
  for (; *s && l && sl; l--, sl--)
    *d++ = *s++;

  *d = '\0';

  return p;
}

void mutt_str_replace (char **p, const char *s)
{
  FREE (p);
  *p = safe_strdup (s);
}

void mutt_str_adjust (char **p)
{
  if (!p || !*p)
    return;
  safe_realloc (p, mutt_strlen (*p) + 1);
}

/* convert all characters in the string to lowercase */
char *mutt_strlower (char *s)
{
  char *p = s;

  while (*p) {
    *p = tolower ((unsigned char) *p);
    p++;
  }

  return (s);
}

/* NULL-pointer aware string comparison functions */

char *mutt_substrcpy (char *dest, const char *beg, const char *end,
                      size_t destlen)
{
  size_t len;

  len = end - beg;
  if (len > destlen - 1)
    len = destlen - 1;
  memcpy (dest, beg, len);
  dest[len] = 0;
  return dest;
}

char *mutt_substrdup (const char *begin, const char *end)
{
  size_t len;
  char *p;

  if (end)
    len = end - begin;
  else
    len = mutt_strlen (begin);

  p = safe_malloc (len + 1);
  memcpy (p, begin, len);
  p[len] = 0;
  return p;
}

int mutt_strcmp (const char *a, const char *b)
{
  return strcmp (NONULL (a), NONULL (b));
}

int mutt_strcasecmp (const char *a, const char *b)
{
  return strcasecmp (NONULL (a), NONULL (b));
}

int mutt_strncmp (const char *a, const char *b, size_t l)
{
  return strncmp (NONULL (a), NONULL (b), l);
}

int mutt_strncasecmp (const char *a, const char *b, size_t l)
{
  return strncasecmp (NONULL (a), NONULL (b), l);
}

size_t mutt_strlen (const char *a)
{
  return a ? strlen (a) : 0;
}

int mutt_strcoll (const char *a, const char *b)
{
  return strcoll (NONULL (a), NONULL (b));
}

const char *mutt_stristr (const char *haystack, const char *needle)
{
  const char *p, *q;

  if (!haystack)
    return NULL;
  if (!needle)
    return (haystack);

  while (*(p = haystack)) {
    for (q = needle;
         *p && *q &&
         tolower ((unsigned char) *p) == tolower ((unsigned char) *q);
         p++, q++);
    if (!*q)
      return (haystack);
    haystack++;
  }
  return NULL;
}

char *mutt_skip_whitespace (char *p)
{
  SKIPWS (p);
  return p;
}

void mutt_remove_trailing_ws (char *s)
{
  char *p;

  for (p = s + mutt_strlen (s) - 1; p >= s && ISSPACE (*p); p--)
    *p = 0;
}

