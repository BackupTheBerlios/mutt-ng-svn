/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 * Copyright (C) 1999-2000 Thomas Roessler <roessler@does-not-exist.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#ifndef _LIB_STR_H
#define _LIB_STR_H

#include <sys/types.h>

#define NONULL(x) x?x:""

# define HUGE_STRING	5120
# define LONG_STRING     1024
# define STRING          256
# define SHORT_STRING    128

/*
 * Create a format string to be used with scanf.
 * To use it, write, for instance, MUTT_FORMAT(HUGE_STRING).
 * 
 * See K&R 2nd ed, p. 231 for an explanation.
 */
# define _MUTT_FORMAT_2(a,b)	"%" a  b
# define _MUTT_FORMAT_1(a, b)	_MUTT_FORMAT_2(#a, b)
# define MUTT_FORMAT(a)		_MUTT_FORMAT_1(a, "s")
# define MUTT_FORMAT2(a,b)	_MUTT_FORMAT_1(a, b)

# define ISSPACE(c) isspace((unsigned char)c)
# define strfcpy(A,B,C) strncpy(A,B,C), *(A+(C)-1)=0

/* this macro must check for *c == 0 since isspace(0) has unreliable behavior
   on some systems */
# define SKIPWS(c) while (*(c) && isspace ((unsigned char) *(c))) c++;

#define ISBLANK(c) (c == ' ' || c == '\t')
/*
 * These functions aren't defined in lib.c, but
 * they are used there.
 *
 * A non-mutt "implementation" (ahem) can be found in extlib.c.
 */

char *mutt_strlower (char *);
char *mutt_substrcpy (char *, const char *, const char *, size_t);
char *mutt_substrdup (const char *, const char *);
char *safe_strcat (char *, size_t, const char *);
char *safe_strncat (char *, size_t, const char *, size_t);
char *safe_strdup (const char *);
int mutt_strcasecmp (const char *, const char *);
int mutt_strcmp (const char *, const char *);
int mutt_strncasecmp (const char *, const char *, size_t);
int mutt_strncmp (const char *, const char *, size_t);
int mutt_strcoll (const char *, const char *);
void mutt_str_replace (char **p, const char *s);
void mutt_str_adjust (char **p);
size_t mutt_strlen (const char *a);

#endif /* !_LIB_STR_H */
