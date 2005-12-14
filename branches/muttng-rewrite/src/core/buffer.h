/** @ingroup core_string */
/**
 * @file core/buffer.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: String buffer
 */
#ifndef MUTTNG_CORE_BUFFER_H
#define MUTTNG_CORE_BUFFER_H

#include <stdlib.h>
#include <ctype.h>
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
 * @test buffer_tests::test_buffer_init().
 */
#define buffer_init(B) do { \
  (B)->str = NULL; \
  (B)->len = 0; \
  (B)->size = 0; \
} while (0)

/**
 * Append initial portion of string to buffer.
 * Memory for buffer and contents will be @c malloc'd as needed.
 * @param buffer Pointer to buffer for storage.
 * @param s Initial portion of string to append.
 * @param len How many characters to consider "initial". If @c -1,
 *        get length of string first via @c str_len().
 * @test buffer_tests::test_buffer_add_str().
 */
void buffer_add_str (buffer_t* buffer, const char* s, int len);

/**
 * Append single character to buffer.
 * Memory for buffer and contents will be @c malloc'd as needed.
 * @param dst Pointer to buffer for storage.
 * @param src Source buffer.
 * @test buffer_tests::test_buffer_add_buffer().
 */
void buffer_add_buffer (buffer_t* dst, buffer_t* src);

/**
 * Append single character to buffer.
 * Memory for buffer and contents will be @c malloc'd as needed.
 * @param buffer Pointer to buffer for storage.
 * @param ch Character to append.
 * @test buffer_tests::test_buffer_add_ch().
 */
void buffer_add_ch (buffer_t* buffer, unsigned char ch);

/**
 * Append number to buffer.
 * Memory for buffer and contents will be @c malloc'd as needed.
 * @param buffer Pointer to buffer for storage.
 * @param num Number to append.
 * @param pad Number of digits for destination. If too few, buffer
 *            will be filled with 0 and truncated otherwise.
 * @param base Number base.
 * @test buffer_tests::test_buffer_add_num2().
 */
void buffer_add_num2 (buffer_t* buffer, int num, int pad, int base);

/**
 * Append number to buffer.
 * Memory for buffer and contents will be @c malloc'd as needed.
 * @param B Pointer to buffer for storage.
 * @param N Number to append.
 * @param P Number of digits for destination. If too few, buffer
 *          will be filled with 0 and truncated otherwise.
 */
#define buffer_add_num(B,N,P)   buffer_add_num2(B,N,P,10)

/**
 * Compare buffer to string.
 * If length of string passed in is negative, it's computed <b>and</b>
 * buffer must match string exactly (length+content). If postive or zero
 * length given, check only first @c slen characters.
 * @param buffer Pointer to buffer.
 * @param s String.
 * @param slen Length of string. If negative, it will be computed first.
 * @return Yes/No.
 * @test buffer_tests::test_buffer_equal1().
 */
int buffer_equal1 (buffer_t* buffer, const char* s, int slen);

/**
 * Compare buffer to buffer
 * @param buffer1 1st buffer.
 * @param buffer2 2nd buffer.
 * @return Yes/No.
 */
int buffer_equal2 (buffer_t* buffer1, buffer_t* buffer2);

/**
 * @c free() all memory consumed for buffer.
 * @b NOTE: This does not @c free() the memory for the buffer itself.
 * @param buffer Pointer to storage of buffer.
 */
void buffer_free (buffer_t* buffer);

/**
 * Mark contents as empty but don't @c free() space.
 * This should be used when using a buffer more than once in a routine
 * with len == 0.
 * @param buffer Pointer to buffer for storage.
 * @param len New length.
 */
void buffer_shrink (buffer_t* buffer, size_t len);

/**
 * Remove trailing CR/LF characters from buffer.
 * @param buffer buffer which shall be chomped.
 * @return number of characters chomped.
 */
unsigned int buffer_chomp(buffer_t * buffer);

#define M_TOKEN_EQUAL           1       /* treat '=' as a special */
#define M_TOKEN_CONDENSE        (1<<1)  /* ^(char) to control chars (macros) */
#define M_TOKEN_SPACE           (1<<2)  /* don't treat whitespace as a term */
#define M_TOKEN_QUOTE           (1<<3)  /* don't interpret quotes */
#define M_TOKEN_PATTERN         (1<<4)  /* !)|~ are terms (for patterns) */
#define M_TOKEN_COMMENT         (1<<5)  /* don't reap comments */
#define M_TOKEN_SEMICOLON       (1<<6)  /* don't treat ; as special */

/**
 * Expand special sequences of source buffer to destination buffer.
 *
 * Special sequences include environment/other variables, control
 * characters, embedded commands via backticks, etc.
 *
 * The implementation is senstive to different quoting types. Special
 * sequences in "-quotes will be expanded while those in ' will just
 * be copied.
 *
 * @param dst Destination buffer.
 * @param token Source buffer.
 * @param flags Flags, see M_TOKEN_*.
 * @param expand Optional callback to call if a variable doesn't seem to
 *               be an environment variable.
 * @return Number of characters parsed from token.
 */
size_t buffer_extract_token (buffer_t* dst, buffer_t* token, int flags,
                             int(*expand)(buffer_t*,buffer_t*));

#ifdef __cplusplus
}
#endif

#endif /* !MUTTNG_BUFFER_H */
