/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#ifndef _HISTORY_H
#define _HISTORY_H

enum history_class {
  HC_CMD,
  HC_ALIAS,
  HC_COMMAND,
  HC_FILE,
  HC_PATTERN,
  HC_OTHER,
  HC_LAST
};

#define HC_FIRST HC_CMD

typedef enum history_class history_class_t;

void mutt_init_history (void);
void mutt_history_add (history_class_t, const char *);
char *mutt_history_next (history_class_t);
char *mutt_history_prev (history_class_t);

#endif
