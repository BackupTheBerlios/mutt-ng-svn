/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 * Copyright (C) 1999-2000 Thomas Roessler <roessler@does-not-exist.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

/* mutt functions which are generally useful. */

#ifndef _LIB_H
# define _LIB_H

# include <stdio.h>
# include <string.h>
# ifdef HAVE_UNISTD_H
#  include <unistd.h>           /* needed for SEEK_SET */
# endif
# include <sys/types.h>
# include <sys/stat.h>
# include <time.h>
# include <limits.h>
# include <stdarg.h>
# include <signal.h>

# ifndef _POSIX_PATH_MAX
#  include <posix1_lim.h>
# endif

# define TRUE 1
# define FALSE 0

# undef MAX
# undef MIN
# define MAX(a,b) ((a) < (b) ? (b) : (a))
# define MIN(a,b) ((a) < (b) ? (a) : (b))

#define FOREVER while (1)

# ifndef _EXTLIB_C
extern void (*mutt_error) (const char *, ...);
# endif
void mutt_exit (int);

/* The actual library functions. */

FILE *safe_fopen (const char *, const char *);

char *mutt_concat_path (char *, const char *, const char *, size_t);
char *mutt_read_line (char *, size_t *, FILE *, int *);
char *mutt_skip_whitespace (char *);

const char *mutt_stristr (const char *, const char *);
const char *mutt_basename (const char *);

int mutt_copy_stream (FILE *, FILE *);
int mutt_copy_bytes (FILE *, FILE *, size_t);
int mutt_rx_sanitize_string (char *, size_t, const char *);

int safe_open (const char *, int);
int safe_symlink (const char *, const char *);
int safe_rename (const char *, const char *);
int safe_fclose (FILE **);

size_t mutt_quote_filename (char *, size_t, const char *);
size_t mutt_strlen (const char *);

void mutt_nocurses_error (const char *, ...);
void mutt_remove_trailing_ws (char *);
void mutt_sanitize_filename (char *, short);
void mutt_unlink (const char *);

#endif
