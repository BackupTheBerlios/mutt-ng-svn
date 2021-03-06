/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "mutt.h"

#include "lib/mem.h"

#define SOMEPRIME 149711

int hash_string (const unsigned char *s, int n)
{
  int h = 0;

#if 0
  while (*s)
    h += *s++;
#else
  while (*s)
    h += (h << 7) + *s++;
  h = (h * SOMEPRIME) % n;
  h = (h >= 0) ? h : h + n;
#endif

  return (h % n);
}

HASH *hash_create (int nelem)
{
  HASH *table = mem_malloc (sizeof (HASH));

  if (nelem == 0)
    nelem = 2;
  table->nelem = nelem;
  table->curnelem = 0;
  table->table = mem_calloc (nelem, sizeof (struct hash_elem *));
  return table;
}

HASH *hash_resize (HASH * ptr, int nelem)
{
  HASH *table;
  struct hash_elem *elem, *tmp;
  int i;

  table = hash_create (nelem);

  for (i = 0; i < ptr->nelem; i++) {
    for (elem = ptr->table[i]; elem;) {
      tmp = elem;
      elem = elem->next;
      hash_insert (table, tmp->key, tmp->data, 1);
      mem_free (&tmp);
    }
  }
  mem_free (&ptr->table);
  mem_free (&ptr);

  return table;
}

/* table        hash table to update
 * key          key to hash on
 * data         data to associate with `key'
 * allow_dup    if nonzero, duplicate keys are allowed in the table 
 */
int hash_insert (HASH * table, const char *key, void *data, int allow_dup)
{
  struct hash_elem *ptr;
  int h;

  ptr = (struct hash_elem *) mem_malloc (sizeof (struct hash_elem));
  h = hash_string ((unsigned char *) key, table->nelem);
  ptr->key = key;
  ptr->data = data;

  if (allow_dup) {
    ptr->next = table->table[h];
    table->table[h] = ptr;
    table->curnelem++;
  }
  else {
    struct hash_elem *tmp, *last;
    int r;

    for (tmp = table->table[h], last = NULL; tmp; last = tmp, tmp = tmp->next) {
      r = str_cmp (tmp->key, key);
      if (r == 0) {
        mem_free (&ptr);
        return (-1);
      }
      if (r > 0)
        break;
    }
    if (last)
      last->next = ptr;
    else
      table->table[h] = ptr;
    ptr->next = tmp;
    table->curnelem++;
  }
  return h;
}

void *hash_find_hash (const HASH * table, int hash, const char *key)
{
  struct hash_elem *ptr = table->table[hash];

  for (; ptr; ptr = ptr->next) {
    if (str_cmp (key, ptr->key) == 0)
      return (ptr->data);
  }
  return NULL;
}

void hash_delete_hash (HASH * table, int hash, const char *key, const void *data,
                       void (*destroy) (void *))
{
  struct hash_elem *ptr = table->table[hash];
  struct hash_elem **last = &table->table[hash];

  while (ptr) {
    if ((data == ptr->data || !data) && str_cmp (ptr->key, key) == 0) {
      *last = ptr->next;
      if (destroy)
        destroy (ptr->data);
      mem_free (&ptr);

      ptr = *last;
    } else {
      last = &ptr->next;
      ptr = ptr->next;
    }
  }
}

/* ptr		pointer to the hash table to be freed
 * destroy()	function to call to free the ->data member (optional) 
 */
void hash_destroy (HASH ** ptr, void (*destroy) (void *))
{
  int i;
  HASH *pptr = *ptr;
  struct hash_elem *elem, *tmp;

  for (i = 0; i < pptr->nelem; i++) {
    for (elem = pptr->table[i]; elem;) {
      tmp = elem;
      elem = elem->next;
      if (destroy)
        destroy (tmp->data);
      mem_free (&tmp);
    }
  }
  mem_free (&pptr->table);
  mem_free (ptr);
}

void hash_map (HASH* table, void (*mapfunc) (const char* key, void* data,
                                             unsigned long more),
               unsigned long more) {
  int i = 0;
  struct hash_elem* elem = NULL;

  if (!table || !mapfunc)
    return;

  for (i = 0; i < table->nelem; i++)
    for (elem = table->table[i]; elem; elem = elem->next)
      mapfunc (elem->key, elem->data, more);
}
