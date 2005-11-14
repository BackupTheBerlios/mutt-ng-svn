/** @ingroup core */
/**
 * @file core/list.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Generic List interface
 */
#ifndef MUTTNG_CORE_LIST_H
#define MUTTNG_CORE_LIST_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/** item type store in list. */
#define LIST_ITEMTYPE   unsigned long

/** Generic datatype for any type of lists */
typedef struct list_t {
  /** array of pointers to actual data */
  LIST_ITEMTYPE* data;
  /** length */
  int length;
} list_t;

/*
 * basics
 */

/** Creates empty list */
list_t* list_new (void);
/**
 * Creates list of given size
 *
 * Useful to avoid malloc() for hash tables and friends.
 */
list_t* list_new2 (int);

/* Free() memory consumed
 *
 * If edel is not @c NULL, it'll be used to free() items
 * as well
 */
typedef void list_del_t (LIST_ITEMTYPE*);
void list_del (list_t**, list_del_t* del);

/** shorthand for checking whether list is empty */
#define list_empty(l) (!l || l->length == 0 || !l->data)

/*
 * insertion, removal
 * the list_push_* functions create a list if empty so far
 * for convenience
 */

/**
 * Append item.
 *
 * Also creates a list if not done yet
 */
void list_push_back (list_t**, LIST_ITEMTYPE);
/**
 * Prepend item.
 *
 * Also creates a list if not done yet
 */
void list_push_front (list_t**, LIST_ITEMTYPE);
/** Remove and return last item */
LIST_ITEMTYPE list_pop_back (list_t*);
/** Remove and return first item */
LIST_ITEMTYPE list_pop_front (list_t*);
/** Remove and return given item */
LIST_ITEMTYPE list_pop_idx (list_t*, int);

/*
 * copying
 */

/** plain copy of pointers */
list_t* list_cpy (list_t*);
/** duplicate ("hard copy") using callback to copy items */
list_t* list_dup (list_t*, LIST_ITEMTYPE (*dup) (LIST_ITEMTYPE));

/*
 * misc
 */

/** Lookup item. */
int list_lookup (list_t*, int (*cmp) (const LIST_ITEMTYPE, const LIST_ITEMTYPE), const LIST_ITEMTYPE);
/** map function to all items */
int list_map (list_t*, int (*) (LIST_ITEMTYPE));

#ifdef __cplusplus
}
#endif

#endif /* !MUTTNG_CORE_LIST_H */
