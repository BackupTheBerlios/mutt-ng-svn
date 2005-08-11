/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */
#ifndef _MUTT_LIST_H
#define _MUTT_LIST_H

typedef struct list_t {
    char *data;
      struct list_t *next;
} LIST;

#define mutt_new_list() mem_calloc (1, sizeof (LIST))
void mutt_free_list (LIST **);

LIST *mutt_copy_list (LIST *);

/* add an element to a list */
LIST *mutt_add_list (LIST*, const char*);
LIST *mutt_add_list_n (LIST*, const void*, size_t len);

#endif /* !_MUTT_LIST_H */
