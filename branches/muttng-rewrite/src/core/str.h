/**
 * @ingroup core
 */
/**
 * @file core/str.h
 * @author Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 * @author Copyright (C) 1999-2000 Thomas Roessler <roessler@does-not-exist.org>
 * @brief Sanity string handling interface
 */
#ifndef MUTTNG_CORE_STR_H
#define MUTTNG_CORE_STR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>

#define NONULL(x) x?x:""

# define HUGE_STRING     5120
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
# define ISBLANK(c) (c == ' ' || c == '\t')
# define strfcpy(A,B,C) strncpy(A,B,C), *(A+(C)-1)=0
/* this macro must check for *c == 0 since isspace(0) has
 * unreliable behavior on some systems */
# define SKIPWS(c) while (*(c) && isspace ((unsigned char) *(c))) c++;

/*
 * safety wrappers/replacements
 * (mostly only difference: safely handle NULL strings)
 */
char *str_dup (const char*);
char *str_cat (char*, size_t, const char*);
char *str_ncat (char*, size_t, const char*, size_t);
int str_cmp (const char*, const char*);
int str_casecmp (const char*, const char*);
int str_ncmp (const char*, const char*, size_t);
int str_ncasecmp (const char*, const char*, size_t);
int str_coll (const char*, const char*);
size_t str_len (const char*);

/*
 * tools
 */
char *str_tolower (char*);
char *str_substrcpy (char*, const char*, const char*, size_t);
char *str_substrdup (const char*, const char*);
void str_replace (char**, const char*);
void str_adjust (char**);
int str_eq (const char*, const char*);
const char *str_isstr (const char*, const char*);
char* str_skip_initws (char*);
void str_skip_trailws (char*);

#ifdef __cplusplus
}
#endif

#endif /* !MUTTNG_CORE_STR_H */
