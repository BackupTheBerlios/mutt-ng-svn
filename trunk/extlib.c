/*
 * Copyright notice from original mutt:
 * Copyright (C) 1999-2000 Thomas Roessler <roessler@does-not-exist.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

/* 
 * Some simple dummies, so we can reuse the routines from
 * lib.c in external programs.
 */

#define WHERE
#define _EXTLIB_C

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include "lib.h"

void (*mutt_error) (const char *, ...) = mutt_nocurses_error;

void mutt_exit (int code)
{
  exit (code);
}
