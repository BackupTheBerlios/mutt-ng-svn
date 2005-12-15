
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

/**
 * Callback for Hash::map(): print value:key.
 * @param key Key.
 * @param argc Value.
 * @param counter Pointer to integer for internal use.
 */
static void hash_print (const char* key, int argc, int* counter) {
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
  std::cout<<tmp.str<<std::endl;
}

/**
 * Callback for Hash::map(): count total length of keys.
 * @param key Key.
 * @param argc Unused.
 * @param counter Pointer to int where result is stored.
 */
static void hash_count (const char* key, int argc, int* counter) {
  (void) argc;
  *counter += str_len(key);
}

/**
 * Callback for Hash::map(): build vector with all keys.
 * @param key Key.
 * @param argc Unused.
 * @param v Pointer to destination vector.
 */
static void hash_keys (const char* key, int argc,
                       std::vector<const char*>* v) {
  (void) argc;
  v->push_back(key);
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
  hash->map<int*>(hash_print,&count);
  hash->map<std::vector<const char*>* >(hash_keys,keys);
  std::cout<<"Have "<<argc<<" arguments and "<<count<<"/"<<
    keys->size()<<" in table."<<std::endl;

  /* count total length of all keys */
  count=0;
  hash->map<int*>(hash_count,&count);
  std::cout<<"Total length is "<<count<<" byte."<<std::endl;

  delete keys;
  delete hash;

  return 0;
}
