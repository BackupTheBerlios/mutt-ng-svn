/*
 * Copyright notice from original mutt:
 * Copyright (C) ????-2004 Justin Hibbits <jrh29@po.cwru.edu>
 * Copyright (C) 2004 Thomer M. Gil <mutt@thomer.com>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#ifndef _SIDEBAR_H
#define _SIDEBAR_H

int sidebar_draw (int);
void sidebar_scroll (int, int);
void sidebar_set_buffystats (CONTEXT *);
const char* sidebar_get_current (void);
void sidebar_set_current (const char*);

#endif /* _SIDEBAR_H */
