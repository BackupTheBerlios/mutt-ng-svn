/*
 * Copyright notice from original mutt:
 * Copyright (C) ????-2004 Justin Hibbits <jrh29@po.cwru.edu>
 * Copyright (C) 2004 Thomer M. Gil <mutt@thomer.com>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#ifndef SIDEBAR_H
#define SIDEBAR_H

/* parameter is whether or not to go to the status line */
/* used for omitting the last | that covers up the status bar in the index */
int draw_sidebar (int);
void scroll_sidebar (int, int);
void set_curbuffy (char *);
void set_buffystats (CONTEXT *);

#endif /* SIDEBAR_H */
