/** @ingroup core */
/**
 * @file core/list.c
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: Generic List
 */
#include <stddef.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include "mem.h"
#include "list.h"

list_t* list_new (void) {
  return (list_new2 (0));
}

list_t* list_new2 (int size) {
  list_t* ret = mem_calloc (1, sizeof (list_t));
  ret->length = size;
  if (size == 0)
    return (ret);
  ret->data = mem_calloc (size, sizeof (LIST_ITEMTYPE));
  return (ret);
}

void list_del (list_t** l, list_del_t* del) {
  int i = 0;
  if (!l || !*l)
    return;
  if (del)
    for (i = 0; i < (*l)->length; i++)
      del (&(*l)->data[i]);
#ifndef LINUX
  /* the only linux system I have is some Knoppix where this line
   * makes trouble when build with:
   *   gcc-Version 3.3.3 20040110 (prerelease) (Debian)
   * although it works with CC="diet gcc" on the very same system
   */
  mem_free (&((*l)->data));
#endif
  mem_free (l);
}

void list_push_back (list_t** l, LIST_ITEMTYPE p) {
  if (!*l)
    *l = list_new ();
  mem_realloc (&(*l)->data, (++(*l)->length)*sizeof(LIST_ITEMTYPE));
  (*l)->data[(*l)->length-1] = p;
}

void list_push_front (list_t** l, LIST_ITEMTYPE p) {
  if (!*l)
    *l = list_new ();
  mem_realloc (&(*l)->data, (++(*l)->length)*sizeof(LIST_ITEMTYPE));
  if ((*l)->length > 1)
    memmove (&(*l)->data[1], &(*l)->data[0], ((*l)->length-1)*sizeof(LIST_ITEMTYPE));
  (*l)->data[0] = p;
}

LIST_ITEMTYPE list_pop_back (list_t* l) {
  LIST_ITEMTYPE p = 0;
  if (list_empty(l))
    return (0);
  p = l->data[l->length-1];
  mem_realloc (&l->data, --(l->length)*sizeof(LIST_ITEMTYPE));
  return (p);
}

LIST_ITEMTYPE list_pop_front (list_t* l) {
  LIST_ITEMTYPE p = 0;
  if (list_empty(l))
    return (0);
  p = l->data[0];
  memmove (&l->data[0], &l->data[1], (--(l->length))*sizeof(LIST_ITEMTYPE));
  mem_realloc (&l->data, l->length*sizeof(LIST_ITEMTYPE));
  return (p);
}

LIST_ITEMTYPE list_pop_idx (list_t* l, int c) {
  LIST_ITEMTYPE p = 0;
  if (list_empty(l) || c < 0 || c >= l->length)
    return (0);
  if (c == l->length-1)
    return (list_pop_back (l));
  p = l->data[c];
  memmove (&l->data[c], &l->data[c+1], (l->length-c)*sizeof(LIST_ITEMTYPE));
  mem_realloc (&l->data, (--(l->length))*sizeof(LIST_ITEMTYPE));
  return (p);
}

list_t* list_cpy (list_t* l) {
  list_t* ret = NULL;
  if (list_empty(l))
    return (NULL);
  ret = list_new ();
  ret->length = l->length;
  ret->data = mem_malloc (l->length*sizeof(LIST_ITEMTYPE));
  memcpy (ret->data, l->data, l->length*sizeof(LIST_ITEMTYPE));
  return (ret);
}

list_t* list_dup (list_t* l, LIST_ITEMTYPE (*dup) (LIST_ITEMTYPE)) {
  list_t* ret = NULL;
  int i = 0;
  if (list_empty(l) || !*dup)
    return (NULL);
  ret = list_new ();
  ret->length = l->length;
  ret->data = mem_malloc (l->length*sizeof(LIST_ITEMTYPE));
  for (i = 0; i < l->length; i++)
    ret->data[i] = dup (l->data[i]);
  return (ret);
}

int list_lookup (list_t* l, int (*cmp) (const LIST_ITEMTYPE, const LIST_ITEMTYPE), const LIST_ITEMTYPE p) {
  int i = 0;
  if (list_empty(l) || !*cmp)
    return (-1);
  for (i = 0; i < l->length; i++)
    if (cmp (l->data[i], p) == 0)
      return (i);
  return (-1);
}
