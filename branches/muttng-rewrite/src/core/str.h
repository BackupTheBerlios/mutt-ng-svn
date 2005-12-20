/**
 * @ingroup core
 * @addtogroup core_string String handling
 * @{
 */
/**
 * @file core/str.h
 * @author Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 * @author Copyright (C) 1999-2000 Thomas Roessler <roessler@does-not-exist.org>
 * @brief Interface: Sanity string handling
 */
#ifndef MUTTNG_CORE_STR_H
#define MUTTNG_CORE_STR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>

/**
 * safety define to avoid segfaults due to @c NULL pointers.
 * @param x String to sanitize.
 */
#define NONULL(x) x?x:""

/**
 * Make string constant of different fractions.
 * I.e. turn _MUTT_FORMAT_2(a,b) into '"%" "10" "s".
 * @param a Size.
 * @param b @c scanf() modifier.
 * @return Quoted string.
 * @bug Legacy: Remove this/Don't use @c scanf().
 */
#define _MUTT_FORMAT_2(a,b)     "%" a  b

/**
 * Make string constant of different fractions.
 * I.e. turn _MUTT_FORMAT_1(a,b) into '"%" "10" "s" by quoting first arg.
 * @param a Size to be quoted.
 * @param b @c scanf() modifier.
 * @return Quoted string.
 * @bug Legacy: Remove this/Don't use @c scanf().
 */
#define _MUTT_FORMAT_1(a, b)    _MUTT_FORMAT_2(#a, b)

/**
 * Create format string for use with @c scanf() to get string.
 * @param a Size of destination string.
 * @return Quoted string.
 * @bug Legacy: Remove this/Don't use @c scanf().
 */
#define MUTT_FORMAT(a)          _MUTT_FORMAT_1(a, "s")

/**
 * Create format string for use with @c scanf() to get something.
 * @param a Size of destination storage.
 * @param b Modifier, eg "s" for string, etc.
 * @return Quoted string.
 * @bug Legacy: Remove this/Don't use @c scanf().
 */
#define MUTT_FORMAT2(a,b)       _MUTT_FORMAT_1(a, b)

/**
 * See if character is a space.
 * param c Character.
 * @return Yes/No.
 */
#define ISSPACE(c) isspace((unsigned char)c)

/**
 * See if character is a blank.
 * @param c Character.
 * @return Yes/No.
 */
#define ISBLANK(c) (c == ' ' || c == '\t')

/**
 * Copy and terminate string.
 * @param A Destination.
 * @param B Source.
 * @param C Size of destination (incl \\0).
 */
#define strfcpy(A,B,C) strncpy(A,B,C), *(A+(C)-1)=0

/**
 * Forward string while character is space.
 * This macro must check for *c == 0 since isspace(0) has
 * unreliable behavior on some systems.
 * @param c String.
 */
#define SKIPWS(c) while (*(c) && isspace ((unsigned char) *(c))) c++;

/**
 * <tt>NULL</tt>-aware @c strdup() wrapper.
 */
char *str_dup (const char* s);
/**
 * <tt>NULL</tt>-aware @c strcat() wrapper.
 */
char *str_cat (char* d, size_t l, const char* s);
/**
 * <tt>NULL</tt>-aware @c strncat() wrapper.
 */
char *str_ncat (char* d, size_t l, const char* s, size_t sl);
/**
 * <tt>NULL</tt>-aware @c strcmp() wrapper.
 */
#define str_cmp(A,B)            strcmp(NONULL(A),NONULL(B))
/**
 * <tt>NULL</tt>-aware @c strcasecmp() wrapper.
 */
#define str_casecmp(A,B)        strcasecmp(NONULL(A),NONULL(B))
/**
 * <tt>NULL</tt>-aware @c strncmp() wrapper.
 */
#define str_ncmp(A,B,L)         strncmp(NONULL(A),NONULL(B),L)
/**
 * <tt>NULL</tt>-aware @c strncasecmp() wrapper.
 */
#define str_ncasecmp(A,B,L)     strncasecmp(NONULL(A),NONULL(B),L)
/**
 * <tt>NULL</tt>-aware @c strcoll() wrapper.
 */
#define str_coll(A,B)           strcoll(NONULL(A),NONULL(B))
/**
 * <tt>NULL</tt>-aware @c strlen() wrapper.
 */
#define str_len(A)              (A?strlen(A):0)

/**
 * Convert string to lower case.
 * @param s String.
 * @return String.
 */
char *str_tolower (char* s);
/**
 * Make substring copy of string.
 * @param dest Destination.
 * @param beg Start from where to copy.
 * @param end End until which to copy.
 * @param destlen Copy at most this many bytes.
 * @return Destination.
 */
char *str_substrcpy (char* dest, const char* beg, const char* end, size_t destlen);
/**
 * Make substring copy of string.
 * @param begin Start from where to copy.
 * @param end End until which to copy.
 * @return @c malloc()'d substring.
 */
char *str_substrdup (const char* begin, const char* end);
/**
 * Replace one string by another via @c malloc().
 * @param p Pointer to destination string. This is @c free()'d first.
 * @param s String to replace p with.
 */
void str_replace (char** p, const char* s);
/**
 * @c realloc() string to fits it's length. This may come in handy
 * once the contents was altered to @c free() up some memory.
 * @param p Pointer to string.
 */
void str_adjust (char** p);
/**
 * Test two strings for equality.
 * Total length as well as contents must be identical for success.
 * @param B 1st string.
 * @param A 2nd string.
 * @return Equality or not.
 */
#define str_eq(A,B)             str_eq2(A,B,str_len(B))
/**
 * Test two strings for equality.
 * Total length as well as contents must be identical for success.
 * @param s1 1st string.
 * @param s2 2nd string.
 * @param s2len Length of 2nd string.
 * @return Equality or not.
 */
int str_eq2 (const char* s1, const char* s2, size_t s2len);
/**
 * See if big string contains little string.
 * @param haystack Big string.
 * @param needle Little string.
 */
const char *str_isstr (const char* haystack, const char* needle);
/**
 * Forward while string starts with space.
 * @param s String.
 * @return string.
 */
char* str_skip_initws (char* s);
/**
 * Remove trailing spaces from string.
 * @param s String.
 */
void str_skip_trailws (char* s);

#ifdef __cplusplus
}
#endif

#endif /* !MUTTNG_CORE_STR_H */

/** @} */
