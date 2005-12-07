/**
 * @ingroup core
 * @addtogroup core_data Data structures
 * @{
 */
/**
 * @file core/list.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: Generic List
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

/**
 * Creates empty list.
 * @return Pointer to list.
 */
list_t* list_new (void);

/**
 * Creates list of given size
 * Useful to avoid malloc() for hash tables and friends.
 * @return Pointer to list.
 */
list_t* list_new2 (int size);

/**
 * For readability: typedef of deletion callback.
 */
typedef void list_del_t (LIST_ITEMTYPE*);

/**
 * Free() all memory for list.
 * @param list List to delete.
 * @param del Callback used to free items (if passed.)
 */
void list_del (list_t** list, list_del_t* del);

/** shorthand for checking whether list is empty */
#define list_empty(l) (!l || l->length == 0 || !l->data)

/**
 * Append item.
 * Also creates a list if not done yet.
 * @param list Pointer to list.
 * @param data Data to append.
 */
void list_push_back (list_t** list, LIST_ITEMTYPE data);

/**
 * Prepend item.
 * Also creates a list if not done yet.
 * @param list Pointer to list.
 * @param data Data to append.
 */
void list_push_front (list_t** list, LIST_ITEMTYPE data);

/**
 * Remove and return last item.
 * Also @c free() list if empty afterwards.
 * @param list List.
 * @return Item (if any.)
 * @test list_tests::test_pop_back().
 */
LIST_ITEMTYPE list_pop_back (list_t** list);

/**
 * Remove and return first item.
 * Also @c free() list if empty afterwards.
 * @param list List.
 * @return Item (if any.)
 * @test list_tests::test_pop_front().
 */
LIST_ITEMTYPE list_pop_front (list_t** list);

/**
 * Remove and return specific item.
 * Also @c free()'s list if empty afterwards.
 * @param list List.
 * @param idx Index at which to pop.
 * @return Item (if any.)
 * @test list_tests::test_pop_idx().
 */
LIST_ITEMTYPE list_pop_idx (list_t** list, int idx);

/**
 * Copy list by copying pointers only.
 * @param list List.
 * @return Copy.
 */
list_t* list_cpy (list_t* list);

/**
 * Duplicate ("hard copy") using callback to copy items.
 * @param list List.
 * @param dup Function to copy items.
 * @return Copy.
 */
list_t* list_dup (list_t* list, LIST_ITEMTYPE (*dup) (LIST_ITEMTYPE));

/**
 * Lookup item.
 * @param list List.
 * @param cmp Comparing function for items. It must return 0 on equality.
 * @param a Item to lookup.
 * @return Index of item in list or negative if not found.
 */
int list_lookup (list_t* list, int (*cmp) (const LIST_ITEMTYPE, const LIST_ITEMTYPE),
                 const LIST_ITEMTYPE a);

#ifdef __cplusplus
}
#endif

#endif /* !MUTTNG_CORE_LIST_H */

/** @} */
