/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

/*
 * A (more) generic interface to regular expression matching
 */

#ifndef MUTT_REGEX_H
#define MUTT_REGEX_H

#ifdef USE_GNU_REGEX
#include "_regex.h"
#else
#include <regex.h>
#endif

/* this is a non-standard option supported by Solaris 2.5.x which allows
 * patterns of the form \<...\>
 */
#ifndef REG_WORDS
#define REG_WORDS 0
#endif

#define REGCOMP(X,Y,Z) regcomp(X, Y, REG_WORDS|REG_EXTENDED|(Z))
#define REGEXEC(X,Y) regexec(&X, Y, (size_t)0, (regmatch_t *)0, (int)0)

typedef struct {
  char *pattern;                /* printable version */
  regex_t *rx;                  /* compiled expression */
  int not;                      /* do not match */
} REGEXP;

WHERE REGEXP Mask;
WHERE REGEXP QuoteRegexp;
WHERE REGEXP ReplyRegexp;
WHERE REGEXP Smileys;
WHERE REGEXP GecosMask;
WHERE REGEXP StripWasRegexp;

#endif /* MUTT_REGEX_H */
