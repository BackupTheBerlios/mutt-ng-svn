/*
 * written for mutt-ng by:
 * Rocco Rutte <pdmef@cs.tu-berlin.de>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#include <stddef.h>
#include <string.h>

#include "list.h"

#include "mem.h"

list2_t* list_new (void) {
  return (safe_calloc (sizeof (list2_t), 1));
}

void list_del (list2_t** l, void (*edel) (void**)) {
  size_t i = 0;
  if (!l || !*l)
    return;
  if (*edel)
    for (i = 0; i < (*l)->length; i++)
      edel (&(*l)->data[i]);
  FREE(&(*l)->data);
  FREE(l);
}

void list_push_back (list2_t** l, void* p) {
  if (!*l)
    *l = list_new ();
  safe_realloc (&(*l)->data, (++(*l)->length)*sizeof(void*));
  (*l)->data[(*l)->length-1] = p;
}

void list_push_front (list2_t** l, void* p) {
  if (!*l)
    *l = list_new ();
  safe_realloc (&(*l)->data, (++(*l)->length)*sizeof(void*));
  if ((*l)->length > 1)
    memmove (&(*l)->data[1], &(*l)->data[0], ((*l)->length-1)*sizeof(void*));
  (*l)->data[0] = p;
}

void* list_pop_back (list2_t* l) {
  void* p = NULL;
  if (list_empty(l))
    return (NULL);
  p = l->data[l->length-1];
  safe_realloc (&l->data, --(l->length)*sizeof(void*));
  return (p);
}

void* list_pop_front (list2_t* l) {
  void* p = NULL;
  if (list_empty(l))
    return (NULL);
  p = l->data[0];
  memmove (&l->data[0], &l->data[1], (--(l->length))*sizeof(void*));
  safe_realloc (&l->data, l->length*sizeof(void*));
  return (p);
}

void* list_pop_idx (list2_t* l, int c) {
  void* p = NULL;
  if (list_empty(l) || c < 0 || c >= l->length)
    return (NULL);
  if (c == l->length-1)
    return (list_pop_back (l));
  p = l->data[c];
  memmove (&l->data[c], &l->data[c+1], (l->length-c)*sizeof(void*));
  safe_realloc (&l->data, (--(l->length))*sizeof(void*));
  return (p);
}

list2_t* list_cpy (list2_t* l) {
  list2_t* ret = NULL;
  if (list_empty(l))
    return (NULL);
  ret = list_new ();
  ret->length = l->length;
  ret->data = safe_malloc (l->length*sizeof(void*));
  memcpy (ret->data, l->data, l->length*sizeof(void*));
  return (ret);
}

list2_t* list_dup (list2_t* l, void* (*dup) (void*)) {
  list2_t* ret = NULL;
  int i = 0;
  if (list_empty(l) || !*dup)
    return (NULL);
  ret = list_new ();
  ret->length = l->length;
  ret->data = safe_malloc (l->length*sizeof(void*));
  for (i = 0; i < l->length; i++)
    ret->data[i] = dup (l->data[i]);
  return (ret);
}

int _list_lookup (list2_t* l, int (*cmp) (const void*, const void*), const void* p) {
  int i = 0;
  if (list_empty(l) || !*cmp)
    return (-1);
  for (i = 0; i < l->length; i++)
    if (cmp (l->data[i], p) == 0)
      return (i);
  return (-1);
}
