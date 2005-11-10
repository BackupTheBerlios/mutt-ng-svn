/**
 * @ingroup core
 */
/**
 * @file core/buffer.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief String buffer interface
 */
#ifndef MUTTNG_CORE_BUFFER_H
#define MUTTNG_CORE_BUFFER_H

#include <stdlib.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/** string buffer data structure */
typedef struct {
  /** data */
  char* str;
  /** how many bytes have been allocated */
  size_t size;
  /** length currently stored */
  size_t len;
} buffer_t;

/**
 * Initialize buffer to zero.
 * @param B Pointer to buffer.
 */
#define buffer_init(B) do { \
  B->str = NULL; \
  B->len = 0; \
  B->size = 0; \
} while (0)

/**
 * Append initial portion of string to buffer.
 * Memory for buffer and contents will be @c malloc'd as needed.
 * @param buffer Pointer to pointer to buffer for storage.
 * @param s Initial portion of string to append.
 * @param len How many characters to consider "initial". If @c -1,
 *        get length of string first via @c str_len().
 */
void buffer_add_str (buffer_t* buffer, const char* s, int len);

/**
 * Append single character to buffer.
 * Memory for buffer and contents will be @c malloc'd as needed.
 * @param buffer Pointer to pointer to buffer for storage.
 * @param ch Character to append.
 */
void buffer_add_ch (buffer_t* buffer, unsigned char ch);

/**
 * Append number to buffer.
 * Memory for buffer and contents will be @c malloc'd as needed.
 * @param buffer Pointer to pointer to buffer for storage.
 * @param num Number to append.
 */
void buffer_add_num (buffer_t* buffer, int num);

/**
 * @c free() all memory consumed for buffer.
 * @b NOTE: This does not @c free() the memory for the buffer itself.
 * @param buffer Pointer to pointer to storage of buffer.
 */
void buffer_free (buffer_t* buffer);

#ifdef __cplusplus
}
#endif

#endif /* !MUTTNG_BUFFER_H */
