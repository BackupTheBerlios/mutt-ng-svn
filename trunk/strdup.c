/*
 * Copyright notice from original mutt:
 * [none]
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

/* ultrix doesn't have strdup */

#include <string.h>
#include <stdlib.h>

char *strdup (const char *s)
{                               /* __MEM_CHECKED__ */
  char *d;

  if (s == NULL)
    return NULL;

  if ((d = malloc (strlen (s) + 1)) == NULL)    /* __MEM_CHECKED__ */
    return NULL;

  memcpy (d, s, strlen (s) + 1);
  return d;
}
