/*
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

/*
 * this is an internal abstraction layer for regular expressions
 */

#ifndef _LIB_RX_H
#define _LIB_RX_H

#include <sys/types.h>
#ifdef USE_GNU_REGEX
#include "_regex.h"
#else
#include <regex.h>
#endif

#include "list.h"

/* this is a non-standard option supported by Solaris 2.5.x which allows
 * patterns of the form \<...\>
 */
#ifndef REG_WORDS
#define REG_WORDS 0
#endif

typedef struct rx_t {
  char *pattern;                /* printable version */
  regex_t *rx;                  /* compiled expression */
  int not : 1;                  /* do not match */
} rx_t;

void rx_free (rx_t**);
rx_t* rx_compile (const char*, int);

/* for handling lists */
int rx_compare (const rx_t*, const rx_t*);      /* compare two patterns */
int rx_list_match (list2_t*, const char*);      /* match all items list agains string */
int rx_lookup (list2_t*, const char*);          /* lookup pattern */

#define REGCOMP(X,Y,Z) regcomp(X, Y, REG_WORDS|REG_EXTENDED|(Z))
#define REGEXEC(X,Y) regexec(X, Y, (size_t)0, (regmatch_t *)0, (int)0)

#endif /* !_LIB_RX_H */
