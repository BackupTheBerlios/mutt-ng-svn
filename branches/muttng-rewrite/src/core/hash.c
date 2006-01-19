/** @ingroup core_data */
/**
 * @file core/hash.c
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: Generic Hash Table
 *
 * This file is published under the GNU Lesser General Public License.
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
  unsigned long size;
  /** number of items */
  unsigned long fill;
  /** rows */
  list_t** rows;
  /** whether key was copied */
  unsigned int dup_key:1;
  /**
   * Whether insertion is locked. We need to do this while we're running
   * hash_map() to not mess with list size etc.
   */
  unsigned int locked:1;
} hash_t;

void* hash_new (unsigned long size, int dup_key) {
  hash_t* ret = mem_calloc (1, sizeof (hash_t));

  ret->size = size;
  ret->rows = mem_calloc (size, sizeof (list_t*));
  ret->dup_key = dup_key;
  return (ret);
}

void hash_destroy (void** t, hash_del_t* destroy) {
  unsigned long r,c;
  hash_t** tab = (hash_t**) t;

  if (!tab || !*tab) return;
  if ((*tab)->locked) return;

  for (r = 0; r < (*tab)->size; r++)
    if ((*tab)->rows && !list_empty((*tab)->rows[r])) {
      for (c = 0; c < (unsigned long)(*tab)->rows[r]->length; c++) {
        hash_item_t* t = (hash_item_t*) (*tab)->rows[r]->data[c];
        if ((*tab)->dup_key)
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
static int _hash_exists (void* t, unsigned long row, const char* key) {
  unsigned long i;
  hash_t* tab = (hash_t*) t;

  if (!tab || row >= tab->size || list_empty(tab->rows[row]) || !key)
    return (-1);
  for (i = 0; i < tab->rows[row]->length; i++) {
#define cur ((hash_item_t*) tab->rows[row]->data[i])
    /* don't use str_eq as it would always compute lengths */
    if (str_eq2 (key, cur->key, cur->keylen))
      return (i);
#undef cur
  }
  return (-1);
}

int hash_add_hash (void* t, const char* key,
                   HASH_ITEMTYPE data, unsigned int code) {
  int row = 0;
  hash_t* tab = (hash_t*) t;
  hash_item_t* item = NULL;

  if (!tab || tab->locked || !key)
    return (0);

  row = MOD(code, tab->size);
  if (_hash_exists (tab, row, key) < 0) {
    item = mem_malloc (sizeof (hash_item_t));
    if (tab->dup_key)
      item->key = strdup (key);
    else
      item->key = (char*) key;
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
static HASH_ITEMTYPE _hash_find_hash (void* t, const char* key, int remove,
                                      unsigned int code) {
  int row = 0, col = 0;
  hash_t* tab = (hash_t*) t;
  HASH_ITEMTYPE ret;

  if (!tab || !key) return (0);
  if (tab->locked && remove) return 0;

  row = MOD(code, tab->size);
  if ((col = _hash_exists (tab, row, key)) < 0)
    return (0);
  if (remove) {
    hash_item_t* tmp = (hash_item_t*) list_pop_idx (&tab->rows[row], col);
    ret = tmp->data;
    if (tab->dup_key)
      mem_free (&tmp->key);
    mem_free (&tmp);
    tab->fill--;
  } else
    ret = ((hash_item_t*) tab->rows[row]->data[col])->data;
  return (ret);
}

int hash_exists_hash (void* t, const char* key, unsigned int code) {
  hash_t* tab = (hash_t*) t;

  if (!tab || !key)
    return (0);
  return (_hash_exists (tab, MOD(code, tab->size), key) >= 0);
}

HASH_ITEMTYPE hash_find_hash (void* t, const char* key, unsigned int code) {
  return (_hash_find_hash (t, key, 0, code));
}

HASH_ITEMTYPE hash_del_hash (void* t, const char* key, unsigned int code) {
  return (_hash_find_hash (t, key, 1, code));
}

/**
 * For sorting items prior to calling hash_map()'s callback: compare two
 * keys.
 * @param a Typecasted pointer to 1st hash_item_t structure.
 * @param b Typecasted pointer to 2nd hash_item_t structure.
 * @return str_ncmp(a,b)
 */
static int itemcmp (const void* a, const void* b) {
  hash_item_t** i1 = (hash_item_t**)a;
  hash_item_t** i2 = (hash_item_t**)b;
  return str_ncmp((*i1)->key,(*i2)->key,(*i1)->keylen);
}

unsigned long hash_map (void* t, int sort, int (*map) (const char*, HASH_ITEMTYPE, unsigned long), unsigned long moredata) {
  unsigned long r,c,ret=0;
  hash_t* tab = (hash_t*) t;
  list_t* tmp = NULL;

  if (!tab)
    return 0;

  /* if no map function given, we're done already */
  if (!map)
    return tab->fill;

  /* create list of fixed size and fill it */
  tmp = list_new2(tab->fill);
  for (r = 0; r < tab->size; r++)
    if (!list_empty(tab->rows[r]))
      for (c = 0; c < tab->rows[r]->length; c++)
        tmp->data[ret++] = (LIST_ITEMTYPE)tab->rows[r]->data[c];
  /* sort list if requested */
  if (sort)
    qsort(tmp->data,tmp->length,sizeof(HASH_ITEMTYPE),itemcmp);
  /* now run callback while it succeeds */
  tab->locked = 1;
  for (r = 0, ret = 0; r < tmp->length; r++, ret++)
#define cur ((hash_item_t*) tmp->data[r])
    if (!map(cur->key,cur->data,moredata))
      break;
#undef cur
  tab->locked = 0;
  list_del(&tmp,NULL);
  return ret;
}

int hash_lock (void* table) {
  int rc = 0;
  hash_t* tab = (hash_t*) table;
  if (tab) {
    rc = !tab->locked;
    tab->locked=1;
  }
  return rc;
}

int hash_unlock (void* table) {
  int rc = 0;
  hash_t* tab = (hash_t*) table;
  if (tab) {
    rc = tab->locked;
    tab->locked=0;
  }
  return rc;
}
