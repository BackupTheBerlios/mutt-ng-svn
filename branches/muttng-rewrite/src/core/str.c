/** @ingroup core_string */
/**
 * @file core/str.c
 * @author Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 * @author Copyright (C) 1999-2000 Thomas Roessler <roessler@does-not-exist.org>
 * @brief Implementation: Sanity string handling
 *
 * This file is published under the GNU General Public License.
 */
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "str.h"

#include "mem.h"

char *str_dup (const char *s)
{
  char *p;
  size_t l;

  if (!s || !*s)
    return 0;
  l = str_len (s) + 1;
  p = (char *) mem_malloc (l);
  memcpy (p, s, l);
  return (p);
}

char *str_cat (char *d, size_t l, const char *s)
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

char *str_ncat (char *d, size_t l, const char *s, size_t sl)
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

void str_replace (char **p, const char *s)
{
  mem_free (p);
  *p = str_dup (s);
}

void str_adjust (char **p)
{
  if (!p || !*p)
    return;
  mem_realloc (p, str_len (*p) + 1);
}

/* convert all characters in the string to lowercase */
char *str_tolower (char *s)
{
  char *p = s;

  while (*p) {
    *p = tolower ((unsigned char) *p);
    p++;
  }

  return (s);
}

/* NULL-pointer aware string comparison functions */

char *str_substrcpy (char *dest, const char *beg, const char *end,
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

char *str_substrdup (const char *begin, const char *end)
{
  size_t len;
  char *p;

  if (end)
    len = end - begin;
  else
    len = str_len (begin);

  p = mem_malloc (len + 1);
  memcpy (p, begin, len);
  p[len] = 0;
  return p;
}

const char *str_isstr (const char *haystack, const char *needle)
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

int str_eq2 (const char* s1, const char* s2, size_t s2len) {
  size_t s1len = str_len (s1);

  if (s1len != s2len)
    return (0);
  return (str_ncmp (s1, s2, s1len) == 0);
}

char* str_skip_initws (char* s) {
  SKIPWS (s);
  return (s);
}

void str_skip_trailws (char *s) {
  char *p;

  for (p = s + str_len (s) - 1; p >= s && ISSPACE (*p); p--)
    *p = 0;
}
