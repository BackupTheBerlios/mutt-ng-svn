
/**
 * @file examples/libmuttng_hash.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Example: Hash table
 */
#include <iostream>
#include <vector>

#include "libmuttng/util/hash.h"
#include "core/str.h"
#include "core/buffer.h"

/** how many byte we reserve to demonstrate aborting Hash::map(). */
#define MAX     128

/** Range definition for use with aborting demonstration. */
typedef struct range_t {
  /** how many items were processed so far */
  unsigned long cur;
  /** how many items at most to process */
  unsigned long max;
  /** how many bytes have been processed */
  unsigned long bytes;
  /** initialize range_t structure. @param max_ Maximum byte count. */
  range_t(unsigned long max_) { cur=0; max=max_; bytes=0; }
} range_t;

/**
 * Callback for Hash::map(): print value:key.
 * @param key Key.
 * @param argc Value.
 * @param counter Pointer to integer for internal use.
 * @return @c true.
 */
static bool hash_print (const char* key, int argc, int* counter) {
  /* increase counter */
  (*counter)++;

  buffer_t tmp;
  buffer_init(&tmp);
  /* format output line: '%3d:%s\n' */
  buffer_init(&tmp);
  buffer_add_num(&tmp,argc,3);
  buffer_add_ch(&tmp,':');
  buffer_add_str(&tmp,key,-1);
  /* print */
  std::cout<<"   "<<tmp.str<<std::endl;
  return true;
}

/**
 * Callback for Hash::map(): print first n value:key pairs.
 * @param key Key.
 * @param argc Value.
 * @param range Range definition.
 * @return @c true while in range, false otherwise.
 */
static bool hash_print2 (const char* key, int argc, range_t* range) {

  if (++range->cur > range->max)
    return false;

  size_t len = str_len(key);
  buffer_t tmp;

  buffer_init(&tmp);
  /* format output line: '%3d:%s (%d+%d=%d)\n' */
  buffer_init(&tmp);
  buffer_add_num(&tmp,argc,3);
  buffer_add_ch(&tmp,':');
  buffer_add_str(&tmp,key,-1);
  buffer_add_str(&tmp," (",2);
  buffer_add_num(&tmp,range->bytes,-1);
  buffer_add_ch(&tmp,'+');
  buffer_add_num(&tmp,len,-1);
  buffer_add_ch(&tmp,'=');
  range->bytes+=len;
  buffer_add_num(&tmp,range->bytes,-1);
  buffer_add_ch(&tmp,')');
  /* print */
  std::cout<<"   "<<tmp.str<<std::endl;
  return true;
}

/**
 * Callback for Hash::map(): count total length of keys. Counting
 * continues while total length doesn't exceed MAX bytes.
 * @param key Key.
 * @param argc Unused.
 * @param counter Pointer to int where result is stored.
 * @return @c true if total length below MAX, @c false otherwise
 */
static bool hash_count (const char* key, int argc, int* counter) {
  (void) argc;
  size_t len = str_len(key);
  if (*counter+(int)len > MAX)
    return false;
  *counter += (int)len;
  return true;
}

/**
 * Callback for Hash::map(): build vector with all keys.
 * @param key Key.
 * @param argc Unused.
 * @param v Pointer to destination vector.
 * @return @c true
 */
static bool hash_keys (const char* key, int argc,
                       std::vector<const char*>* v) {
  (void) argc;
  v->push_back(key);
  return true;
}

/**
 * @c main().
 * @param argc Counter for command-line arguments.
 * @param argv Command-line arguments.
 * @return 0 on success, 1 otherwise.
 */
int main(int argc,char** argv) {
  int i,count;
  Hash<int>* hash = new Hash<int>(argc*2);
  std::vector<const char*>* keys = new std::vector<const char*>;

  /* fill hash table */
  for (i=0; i<argc; i++)
    hash->add(argv[i],i);

  /* print all keys and build up vector */
  count=0;
  std::cout<<"-> Arguments:"<<std::endl;
  hash->map<int*>(true,hash_print,&count);
  hash->map<std::vector<const char*>* >(true,hash_keys,keys);
  std::cout<<"-> Have "<<argc<<" arguments and "<<count<<"/"<<
    keys->size()<<" non-duplicate in table."<<std::endl;

  /* count number of first n keys so that total length of keys is <= MAX */
  count=0;
  unsigned long items = hash->map<int*>(true,hash_count,&count);
  std::cout<<"-> First "<<items<<" items fit in "<<MAX<<" byte:"<<std::endl;
  range_t range(items);
  hash->map<range_t*>(true,hash_print2,&range);
  std::cout<<"-> Is total length "<<range.bytes<<" <= "<<MAX<<": "<<
    (range.bytes<=MAX?"yes":"no")<<std::endl;

  delete keys;
  delete hash;

  return 0;
}
