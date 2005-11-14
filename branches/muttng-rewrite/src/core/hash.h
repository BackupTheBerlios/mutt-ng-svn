/** @ingroup core */
/**
 * @file core/hash.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Generic Hash Table Interface.
 */
#ifndef MUTTNG_CORE_HASH_H
#define MUTTNG_CORE_HASH_H

#ifdef __cplusplus
extern "C" {
#endif

/** mathematically correct modulo. */
#define MOD(a,b) (((a % b) + b) % b)

/** data type stored in table. */
#define HASH_ITEMTYPE           unsigned long

/**
 * Hash function for strings.
 * This is the long version for debugging purpose so that we can more
 * easily track keycode computations.
 * @param key Key.
 * @param file Source file.
 * @param line Line in source file where it's called.
 * @return key's code for use with hash_*_hash()
 */
unsigned int hash_key (const char* k);

/**
 * Create new hash table.
 * @param size Row count.
 * @param dup_key If @c 1, copy key and assume constant otherwise.
 * @return Pointer to storage for hash table
 */
void* hash_new (int size, int dup_key);

/**
 * Free all memory allocated for table.
 * @param table Hash table.
 * @param destroy Function callback used to free data if not @c NULL
 */
void hash_destroy (void** table, void (*destroy) (HASH_ITEMTYPE*));

/**
 * Add item.
 * @param table Hash table.
 * @param key Key.
 * @param data Data pointer.
 * @param keycode Already computed code for key.
 * @return Whether item could be inserted or not, i.e. existed or not.
 */
int hash_add_hash (void* table, const void* key, HASH_ITEMTYPE data, unsigned int keycode);

/**
 * convencience wrapper around hash_add_hash().
 * Use with string keys only.
 */
#define hash_add(tab,key,data) hash_add_hash(tab,key,data,hash_key(key))

/**
 * Delete and return item.
 * @param table Hash table.
 * @param key Key to find.
 * @param keycode Already computed code for key.
 * @return Data associated with key if any.
 */
HASH_ITEMTYPE hash_del_hash (void* table, const void* key, unsigned int keycode);

/**
 * convencience wrapper around hash_del_hash().
 * Use with string keys only.
 */
#define hash_del(tab,key) hash_del_hash(tab,key,hash_key(key))

/**
 * Look up and return data associated with key
 * @param table Hash table.
 * @param key Key to find.
 * @param keycode Already computed code for key.
 * @return Data associated with key if any.
 */
HASH_ITEMTYPE hash_find_hash (void* table, const void* key, unsigned int keycode);

/**
 * convencience wrapper around hash_find_hash().
 * Use with string keys only.
 */
#define hash_find(tab,key) hash_find_hash(tab,key,hash_key(key))

/**
 * Test if key exists.
 * This is necessary as tables may be used to manage key only, i.e. no
 * data so that hash_find_hash() always returns 0/@c NULL.
 * @param table Hash table.
 * @param key Key to find.
 * @param keycode Already computed code for key.
 * @return 1 if key exists and 0 otherwise.
 */
int hash_exists_hash (void* table, const void* key, unsigned int keycode);

/**
 * convencience wrapper around hash_exists_hash().
 * Use with string keys only.
 */
#define hash_exists(tab,key) hash_exists_hash(tab,key,hash_key(key))

/**
 * Map function to all items.
 * Don't assume any order of keys.
 * @param table Hash table.
 * @param map Function callback.
 * @param moredata Additional data passed through.
 */
void hash_map (void* table, void (*map) (const void* key, HASH_ITEMTYPE data,
                                         unsigned long moredata),
               unsigned long moredata);

#ifdef __cplusplus
}
#endif

#endif /* !MUTTNG_CORE_HASH_H */
