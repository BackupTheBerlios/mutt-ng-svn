/** @ingroup libmuttng_util */
/**
 * @file libmuttng/util/hash.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: Typesafe wrapper for hash_t
 *
 * This file is published under the GNU Lesser General Public License.
 */
#ifndef LIBMUTTNG_UTIL_HASH_H
#define LIBMUTTNG_UTIL_HASH_H

#include "core/hash.h"
#include "core/mem.h"

#include "libmuttng/libmuttng.h"

/** empty structure for passing through Hash::map */
struct nil {};

/**
 * Typesafe wrapper for hash_t.
 * @param T Name of type to be stored in hash table.
 */
template<typename T>
class Hash : public LibMuttng {
  private:
    /** for readability */
    typedef void hashdel_t (T*);
    /** destroy function */
    hashdel_t* destroy;
    /** table */
    void* table;
    static inline int getkeys(const char* key, unsigned long unused, unsigned long data) {
      (void) unused;
      std::vector<const char*>* dst = (std::vector<const char*>*) data;
      dst->push_back(key);
      return 1;
    }
  public:
    /**
     * Construct new hash table.
     * @param size Size.
     * @param dup_key Whether duplicate keys are allowed.
     * @param destroy_ Which function to use to free memory upon
     *                 removal of table
     */
    inline Hash(unsigned long size, bool dup_key = false, hashdel_t* destroy_ = NULL) : destroy(destroy_) {
      table = hash_new (size, dup_key);
    }
    /** Destructor */
    inline ~Hash() {
      hash_destroy(&table, (void (*) (HASH_ITEMTYPE*)) destroy);
    }
    /**
     * Add item to table.
     * @param key Key
     * @param data Value
     * @param code Key's hash code.
     * @return Success of insertion.
     */
    inline bool add (const char* key, T data, unsigned int code) {
      return hash_add_hash (table, key, (HASH_ITEMTYPE) data, code);
    }
    /**
     * Add item to table.
     * @param key Key
     * @param data Value
     * @return Success of insertion.
     */
    inline bool add (const char* key, T data) {
      return (add (key, data, hash_key (key)));
    }

    /**
     * Remove item from table.
     * @param key Key
     * @param code Key's hash code.
     * @return Item or @c 0 if not found.
     */
    inline T del (const char* key, unsigned int code) {
      return ((T) hash_del_hash (table, key, code));
    }
    /**
     * Remove item from table.
     * @param key Key
     * @return Item or @c 0 if not found.
     */
    inline T del (const char* key) {
      return (del (key, hash_key (key)));
    }

    /**
     * Lookup item.
     * @param key Key.
     * @param code Key's hash code.
     * @return Item.
     */
    inline T find (const char* key, unsigned int code) {
      return ((T) hash_find_hash(table, key, code));
    }
    /**
     * Lookup item.
     * @param key Key.
     * @return Item.
     */
    inline T find (const char* key) {
      return (find (key, hash_key (key)));
    }

    /**
     * See if item exists.
     * @param key Key.
     * @param code Key's hash code.
     * @return Found or not.
     */
    inline bool exists (const char* key, unsigned int code) {
      return (hash_exists_hash(table, key, code) == 1);
    }
    /**
     * See if item exists.
     * @param key Key.
     * @return Found or not.
     */
    inline bool exists (const char* key) {
      return (exists (key, hash_key (key)));
    }

    /**
     * Map a function to all keys.
     * @param map Map function.
     * @param sort Whether to sort keys.
     * @param moredata Additional data just passed through.
     */
    template<typename D>
    inline unsigned long map (bool sort, bool (*map) (const char* key, T data, D moredata), D moredata) {
      return hash_map(table,sort,(int(*)(const char*,HASH_ITEMTYPE,unsigned long)) map,(unsigned long) moredata);
    }

    /**
     * Get all keys.
     * @param sort Whether to sort keys.
     * @return Keys.
     */
    inline std::vector<const char*>* getKeys (bool sort=true) {
      std::vector<const char*>* ret = new std::vector<const char*>;
      hash_map(table,sort,getkeys,(unsigned long) ret);
      return ret;
    }

    /**
     * Lock hash table so that write operations fail unless unlocked again.
     * @return True if the table was unlocked, false otherwise.
     */
    inline bool lock () { return hash_lock(table)==1; }

    /**
     * Unlock hash table so that write operations may succeed unless locked again.
     * @return True if the table was locked, false otherwise.
     */
    inline bool unlock () { return hash_lock(table)==1; }

};

#endif
