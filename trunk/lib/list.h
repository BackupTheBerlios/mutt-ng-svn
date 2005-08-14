/*
 * written for mutt-ng by:
 * Rocco Rutte <pdmef@cs.tu-berlin.de>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

/*
 * this aims to provide a generic list 
 * implementation using arrays only
 * mostly untested
 */

#ifndef _LIB_LIST_H
#define _LIB_LIST_H

#include <sys/types.h>

typedef struct list2_t {
  void** data;
  size_t length;
} list2_t;

/*
 * basics
 */

list2_t* list_new (void);

typedef void list_del_t (void**);

/* free() all memory used by list and optionally
 * use del function to free() items as well */
void list_del (list2_t**, list_del_t* del);

#define list_empty(l) (!l || l->length == 0 || !l->data)

/*
 * insertion, removal
 * the list_push_* functions create a list if empty so far
 * for convenience
 */
void list_push_back (list2_t**, void*);
void list_push_front (list2_t**, void*);
void* list_pop_back (list2_t*);
void* list_pop_front (list2_t*);
void* list_pop_idx (list2_t*, int);

/*
 * copying
 */

/* plain copy */
list2_t* list_cpy (list2_t*);
/* "hard copy" using callback to copy items */
list2_t* list_dup (list2_t*, void* (*dup) (void*));

/*
 * misc
 */

/* looks up item in list using callback function and comparison item
 * return:
 *      -1 if not found
 *      index in data array otherwise
 * the callback must return 0 on equality
 */
int list_lookup (list2_t*, int (*cmp) (const void*, const void*), const void*);

/*
 * dumb-splits string at boundary characters into list
 */
list2_t* list_from_str (const char* str, const char* delim);

#endif /* !_LIB_LIST_H */
