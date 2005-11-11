#ifndef MUTTNG_CORE_IO_H
#define MUTTNG_CORE_IO_H

/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 * Copyright (C) 1999-2000 Thomas Roessler <roessler@does-not-exist.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

int io_open (const char *path, int flags, int u);
FILE* io_fopen (const char *path, const char *mode, int u);

#ifdef __cplusplus
}
#endif

#endif
