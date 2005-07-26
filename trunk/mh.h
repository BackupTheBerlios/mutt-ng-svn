/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2002 Michael R. Elkins <me@mutt.org>
 * Copyright (C) 1999-2002 Thomas Roessler <roessler@does-not-exist.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

/*
 * functions for dealing with Maildir/MH style mailboxes
 */

#ifndef _MH_H
#define _MH_H

#include <sys/stat.h>

#include "mx.h"

/* TODO all of these must disappear to achieve good information hiding */

int mh_buffy (const char *);
FILE *maildir_open_find_message (const char *, const char *);

/* these are the only publicly visible for usage */

mx_t* maildir_reg_mx (void);
mx_t* mh_reg_mx (void);

#endif /* !_MH_H */
