
/**
 * @file examples/core_hash.c
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Example: Hash table
 */

/* for write(2) */
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include "core/hash.h"
#include "core/buffer.h"

/**
 * Callback for hash_map(): print value::key.
 * @param key Key.
 * @param data Value.
 * @param moredata Pointer to integer for counting.
 * @return 1
 */
static int map_print(const char* key, HASH_ITEMTYPE data,
                     unsigned long moredata) {
  buffer_t tmp;
  int* count = (int*)moredata;

  /* increase counter by 1 */
  (*count)++;

  /* format output line: '%3d:%s\n' */
  buffer_init(&tmp);
  buffer_add_num(&tmp,data,3);
  buffer_add_ch(&tmp,':');
  buffer_add_str(&tmp,key,-1);
  buffer_add_ch(&tmp,'\n');
  /* print */
  write(1,tmp.str,tmp.len);
  return 1;
}

/**
 * @c main().
 * @param argc Counter for command-line arguments.
 * @param argv Command-line arguments.
 * @return 0 on success, 1 otherwise.
 */
int main(int argc,char** argv) {
  int i=0,count=0;
  void* hash = hash_new(argc*2,0);

  /* put all arguments into a hash table */
  for (i=0; i<argc; i++)
    hash_add(hash,argv[i],i);

  /* map the printing function to it */
  hash_map(hash,1,map_print,(unsigned long) &count);

  /*
   * destroy hash table. no need to pass a callback
   * function for destroying the individual items
   * as we just store numbers
   */
  hash_destroy(&hash,NULL);

  return 0;
}
