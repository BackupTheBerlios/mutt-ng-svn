/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */
#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "lib/mem.h"
#include "lib/str.h"

LIST *mutt_copy_list (LIST * p) {
  LIST *t, *r = NULL, *l = NULL;

  for (; p; p = p->next) {
    t = (LIST *) mem_malloc (sizeof (LIST));
    t->data = str_dup (p->data);
    t->next = NULL;
    if (l) {
      r->next = t;
      r = r->next;
    }
    else
      l = r = t;
  }
  return (l);
}


LIST *mutt_add_list (LIST * head, const char *data) {
  size_t len = str_len (data);
  return (mutt_add_list_n (head, data, len ? len + 1 : 0));
}

LIST *mutt_add_list_n (LIST *head, const void *data, size_t len) {
  LIST *tmp;

  for (tmp = head; tmp && tmp->next; tmp = tmp->next);

  if (tmp) {
    tmp->next = mem_malloc (sizeof (LIST));
    tmp = tmp->next;
  } else
    head = tmp = mem_malloc (sizeof (LIST));

  tmp->data = mem_malloc (len);
  if (len)
    memcpy (tmp->data, data, len);
  tmp->next = NULL;
  return head;
}

void mutt_free_list (LIST ** list) {
  LIST *p;

  if (!list)
    return;
  while (*list) {
    p = *list;
    *list = (*list)->next;
    mem_free (&p->data);
    mem_free (&p);
  }
}
