/** @ingroup core */
/**
 * @file core/hash.c
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Generic Hash Table implementation
 */
#include <stdlib.h>
#include <string.h>

#include "mem.h"
#include "list.h"
#include "hash.h"
#include "str.h"

/** Prime number for better distribution of hash codes over table indices. */
#define SOMEPRIME       149711

/** Item in Hash Table */
typedef struct {
  /** key identifier */
  char* key;
  /** length of key */
  size_t keylen;
  /** associated data pointer */
  HASH_ITEMTYPE data;
} hash_item_t;

/**
 * Hash Table Data Structure
 *
 * It simply uses an array of list_t objects. Please try to not
 * refer to items directly. Use the interface functions provided here
 * only.
 * */
typedef struct {
  /** number of rows (constant) */
  int size;
  /** number of items */
  int fill;
  /** rows */
  list_t** rows;
  /** whether key was only copied */
  unsigned int dup_key:1;
} hash_t;

void* hash_new (int size, int dup_key) {
  hash_t* ret = mem_calloc (1, sizeof (hash_t));

  ret->size = size;
  ret->rows = mem_calloc (size, sizeof (list_t*));
  ret->dup_key = dup_key;
  return (ret);
}

void hash_destroy (void** t, void (*destroy) (HASH_ITEMTYPE*)) {
  int r = 0, c = 0;
  hash_t** tab = (hash_t**) t;

  if (!tab || !*tab)
    return;
  for (r = 0; r < (*tab)->size; r++)
    if ((*tab)->rows && !list_empty((*tab)->rows[r])) {
      for (c = 0; c < (*tab)->rows[r]->length; c++) {
        hash_item_t* t = (hash_item_t*) (*tab)->rows[r]->data[c];
        if (!(*tab)->dup_key)
          mem_free (&t->key);
        if (destroy)
          destroy (&t->data);
        mem_free (&t);
      }
      list_del(&(*tab)->rows[r], NULL);
    }
  mem_free (&(*tab)->rows);
  mem_free (tab);
}

unsigned int hash_key (const char* k) {
  register int h = 1;
  register char* p = (char*) k;
  while (*p)
    h += (h << 7) + *p++;
  return (h*SOMEPRIME);
}

/**
 * Checks whether an item exists within a row.
 * @param t hash table
 * @param row row to walk along
 * @param key key to look for
 * @return
 *      - >= 0: column index
 *      - < 0: not found
 */
static int _hash_exists (void* t, int row, const void* key) {
  int i = 0;
  hash_t* tab = (hash_t*) t;

  if (!tab || row < 0 || row >= tab->size || list_empty(tab->rows[row]) || !key)
    return (-1);
  for (i = 0; i < tab->rows[row]->length; i++) {
#define cur ((hash_item_t*) tab->rows[row]->data[i])
    /* don't use str_eq as it would always compute lengths */
    if (str_eq2 (key, cur->key, cur->keylen) == 0)
      return (i);
#undef cur
  }
  return (-1);
}

int hash_add_hash (void* t, const void* key,
                   HASH_ITEMTYPE data, unsigned int code) {
  int row = 0;
  hash_t* tab = (hash_t*) t;
  hash_item_t* item = NULL;

  if (!tab || !key)
    return (0);
  row = MOD(code, tab->size);
  if (_hash_exists (tab, row, key) < 0) {
    item = mem_malloc (sizeof (hash_item_t));
    if (tab->dup_key)
      item->key = strdup (key);
    item->keylen = str_len (key);
    item->data = data;
    list_push_back (&tab->rows[row], (LIST_ITEMTYPE) item);
    tab->fill++;
    return (1);
  }
  return (0);
}

/**
 * Item lookup.
 *
 * @param t Hash Table
 * @param key Key to find
 * @param remove If @c true, the item will be also be removed.
 * @param code Already elsewhere computed keycode.
 *
 * @return What has been found
 */
static HASH_ITEMTYPE _hash_find_hash (void* t, const void* key, int remove,
                                      unsigned int code) {
  int row = 0, col = 0;
  hash_item_t* ret = NULL;
  hash_t* tab = (hash_t*) t;

  if (!tab || !key)
    return (0);
  row = MOD(code, tab->size);
  if ((col = _hash_exists (tab, row, key)) < 0)
    return (0);
  if (remove) {
    ret = (hash_item_t*) list_pop_idx (tab->rows[row], col);
    if (tab->rows[row]->length == 0)
      list_del (&tab->rows[row], (void (*) (HASH_ITEMTYPE*)) _mem_free);
    mem_free (&ret->key);
    tab->fill--;
  } else
    ret = (hash_item_t*) tab->rows[row]->data[col];
  return (ret->data);
}

int hash_exists_hash (void* t, const void* key, unsigned int code) {
  hash_t* tab = (hash_t*) t;

  if (!tab || !key)
    return (0);
  return (_hash_exists (tab, MOD(code, tab->size), key) >= 0);
}

HASH_ITEMTYPE hash_find_hash (void* t, const void* key, unsigned int code) {
  return (_hash_find_hash (t, key, 0, code));
}

HASH_ITEMTYPE hash_del_hash (void* t, const void* key, unsigned int code) {
  return (_hash_find_hash (t, key, 1, code));
}

void hash_map (void* t, void (*map) (const void*, HASH_ITEMTYPE, unsigned long), unsigned long moredata) {
  int r = 0, c = 0;
  hash_t* tab = (hash_t*) t;

  if (!tab || !map)
    return;
  for (r = 0; r < tab->size; r++)
    if (!list_empty(tab->rows[r]))
      for (c = 0; c < tab->rows[r]->length; c++)
#define cur ((hash_item_t*) tab->rows[r]->data[c])
        map (cur->key, cur->data, moredata);
#undef cur
}

/** @} */
