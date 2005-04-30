/*
 * written for mutt-ng by:
 * Rocco Rutte <pdmef@cs.tu-berlin.de>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

/* generic interface for debug messages */

#ifndef _LIB_DEBUG_H
#define _LIB_DEBUG_H

#include <stdio.h>

#ifdef DEBUG

extern short DebugLevel;
extern FILE* DebugFile;

void debug_setlevel (short);
void debug_start (const char*);

void _debug_print_intro (const char*, int, const char*, int);
void _debug_print_msg (const char*, ...);

/*
 * the debug_print() macro will (in the end) print debug messages of the
 * following format:
 *
 *      (file:line:function:level): message
 *
 * for GCC and:
 *
 *      (file:line:level): message
 *
 * otherwise
 */
#ifdef __GNUC__
#define debug_print(level,msg) do { \
  if (DebugLevel >= level) { \
    _debug_print_intro (__FILE__,__LINE__,__FUNCTION__,level); \
    _debug_print_msg msg; \
  } \
} while(0)
#else /* __GNUC__ */
#define debug_print(level,msg) do { \
  if (DebugLevel >= level) { \
    _debug_print_intro (__FILE__,__LINE__,NULL,level); \
    _debug_print_msg msg; \
  } \
} while(0)
#endif /* !__GNUC__ */

#else /* DEBUG */

/*
 * without debug support, we don't need these
 * (this also kills the dozens of #ifdef for debug...
 */
#define debug_start(basedir)
#define debug_setlevel(level)
#define debug_print(level,msg)

#endif /* !DEBUG */

#endif /* !_LIB_DEBUG_H */
