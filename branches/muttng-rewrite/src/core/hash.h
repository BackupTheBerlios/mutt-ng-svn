/** @ingroup core_data */
/**
 * @file core/hash.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: Generic Hash Table
 *
 * This file is published under the GNU Lesser General Public License.
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
 * @param k Key.
 * @return key's code for use with hash_*_hash()
 */
unsigned int hash_key (const char* k);

/**
 * Create new hash table.
 * @param size Row count.
 * @param dup_key If @c 1, copy key and assume constant otherwise.
 * @return Pointer to storage for hash table
 */
void* hash_new (unsigned long size, int dup_key);

/**
 * For readability: typedef of deletion callback.
 */
typedef void hash_del_t (HASH_ITEMTYPE*);

/**
 * Free all memory allocated for table.
 * @param table Hash table.
 * @param destroy Function callback used to free data if not @c NULL
 */
void hash_destroy (void** table, hash_del_t* destroy);

/**
 * Add item.
 * @param table Hash table.
 * @param key Key.
 * @param data Data pointer.
 * @param keycode Already computed code for key.
 * @return Whether item could be inserted or not, i.e. existed or not.
 */
int hash_add_hash (void* table, const char* key, HASH_ITEMTYPE data, unsigned int keycode);

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
HASH_ITEMTYPE hash_del_hash (void* table, const char* key, unsigned int keycode);

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
HASH_ITEMTYPE hash_find_hash (void* table, const char* key, unsigned int keycode);

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
int hash_exists_hash (void* table, const char* key, unsigned int keycode);

/**
 * convencience wrapper around hash_exists_hash().
 * Use with string keys only.
 */
#define hash_exists(tab,key) hash_exists_hash(tab,key,hash_key(key))

/**
 * Map function to all items. Don't assume any order of keys. The
 * mapping continues while the callback returns success. If no map
 * function is given, just count how many times it would be called.
 * @param table Hash table.
 * @param sort Whether keys are to be sorted or not.
 * @param map Optional function callback.
 * @param moredata Additional data passed through.
 * @return Number of items processed.
 */
unsigned long hash_map (void* table, int sort, int (*map) (const char* key,
                                                           HASH_ITEMTYPE data,
                                                           unsigned long moredata),
                        unsigned long moredata);

/**
 * Get number of items in hash table.
 * @param T hash table.
 * @return Items.
 */
#define hash_fill(T)    hash_map(T,0,NULL,0)

/**
 * Lock a table. All write operations will fail until it is unlocked
 * again.
 * @param table Hash table.
 * @return
 *   - 1 if table was unlocked
 *   - 0 if table was already locked
 */
int hash_lock (void* table);

/**
 * Unlock a table. All write operations will work again.
 * @param table Hash table.
 * @return
 *   - 1 if table was locked.
 *   - 0 if table was already unlocked
 */
int hash_unlock (void* table);

#ifdef __cplusplus
}
#endif

#endif /* !MUTTNG_CORE_HASH_H */
