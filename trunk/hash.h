/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#ifndef _HASH_H
#define _HASH_H

struct hash_elem {
  const char *key;
  void *data;
  struct hash_elem *next;
};

typedef struct {
  int nelem, curnelem;
  struct hash_elem **table;
} HASH;

#define hash_find(table, key) hash_find_hash(table, hash_string ((unsigned char *)key, table->nelem), key)

#define hash_delete(table,key,data,destroy) hash_delete_hash(table, hash_string ((unsigned char *)key, table->nelem), key, data, destroy)

HASH *hash_create (int nelem);
int hash_string (const unsigned char *s, int n);
int hash_insert (HASH * table, const char *key, void *data, int allow_dup);
HASH *hash_resize (HASH * table, int nelem);
void *hash_find_hash (const HASH * table, int hash, const char *key);
void hash_delete_hash (HASH * table, int hash, const char *key,
                       const void *data, void (*destroy) (void *));
void hash_destroy (HASH ** hash, void (*destroy) (void *));

#endif
