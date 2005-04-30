/*
 * written for mutt-ng by:
 * Rocco Rutte <pdmef@cs.tu-berlin.de>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */
#if HAVE_CONFIG_H
#include "config.h"
#endif

#if DEBUG

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#include "debug.h"

#include "mutt.h"
#include "globals.h"
#include "str.h"

short DebugLevel = -1;
FILE* DebugFile = NULL;

void debug_setlevel (short level) {
  DebugLevel = level;
}

void debug_start (const char* basedir) {
  time_t t;
  int i;
  char buf[_POSIX_PATH_MAX];
  char buf2[_POSIX_PATH_MAX];

  if (DebugLevel < 0 || !basedir || !*basedir)
    return;
  /* rotate the old debug logs */
  for (i = 3; i >= 0; i--) {
    snprintf (buf, sizeof (buf), "%s/.muttngdebug%d", NONULL (basedir), i);
    snprintf (buf2, sizeof (buf2), "%s/.muttngdebug%d", NONULL (basedir), i + 1);
    rename (buf, buf2);
  }
  if ((DebugFile = safe_fopen (buf, "w")) != NULL) {
    t = time (NULL);
    setbuf (DebugFile, NULL);   /* don't buffer the debugging output! */
    fprintf (DebugFile,
             "Mutt-ng %s started at %s\nDebugging at level %d\n\n",
             MUTT_VERSION, asctime (localtime (&t)), DebugLevel);
  }
}

void _debug_print_intro (const char* file, int line, const char* function, int level) {
  if (!DebugFile || DebugLevel < 0)
    return;
  fprintf (DebugFile, "[%d:%s:%d", level, NONULL(file), line);
  if (function && *function)
    fprintf (DebugFile, ":%s()", function);
  fprintf (DebugFile, "] ");
}

void _debug_print_msg (const char* fmt, ...) {
  va_list ap;

  if (!DebugFile || DebugLevel < 0)
    return;
  va_start (ap, fmt);
  vfprintf (DebugFile, fmt, ap);
  va_end (ap);
}

#endif /* DEBUG */
