/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 * Copyright (C) 1998-2000 Thomas Roessler <roessler@does-not-exist.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#ifndef _DOTLOCK_H
#define _DOTLOCK_H

/* exit values */

#define DL_EX_OK	0
#define DL_EX_ERROR	1
#define DL_EX_EXIST	3
#define DL_EX_NEED_PRIVS 4
#define DL_EX_IMPOSSIBLE 5

/* flags */

#define DL_FL_TRY	(1 << 0)
#define DL_FL_UNLOCK	(1 << 1)
#define DL_FL_USEPRIV	(1 << 2)
#define DL_FL_FORCE	(1 << 3)
#define DL_FL_RETRY	(1 << 4)
#define DL_FL_UNLINK	(1 << 5)

#define DL_FL_ACTIONS (DL_FL_TRY|DL_FL_UNLOCK|DL_FL_UNLINK)

#ifndef DL_STANDALONE
int dotlock_invoke (const char *, int, int, int);
#endif

#endif
