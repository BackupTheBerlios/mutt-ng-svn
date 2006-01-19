/** @ingroup core_string */
/**
 * @file core/buffer_token.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: String buffer
 */
#ifndef MUTTNG_CORE_BUFFER_TOKEN_H
#define MUTTNG_CORE_BUFFER_TOKEN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "buffer_base.h"

/*
 * Prepare buffer for tokenizer. Preparation includes expanding all
 * non-single-quoted commands (in backticks) and variables (with an
 * optional callback for non-environment variables).
 * @param dst Destination buffer.
 * @param src Source buffer.
 * @param expand Optional callback for expand variables not defined
 *               in the current environment.
 * @return Number of chars read.
 * @test buffer_tests::test_buffer_tokenize().
 */
size_t buffer_expand (buffer_t* dst, buffer_t* src,
                      int(*expand)(buffer_t* dst,buffer_t* var));

/** flag for buffer_extract_token(): treat '=' as a special */
#define M_TOKEN_EQUAL           1
/** flag for buffer_extract_token(): ^X to CTRL+X */
#define M_TOKEN_CONDENSE        (1<<1)
/** flag for buffer_extract_token(): don't tread whitespace as a term */
#define M_TOKEN_SPACE           (1<<2)
/** flag for buffer_extract_token(): don't interpret quotes */
#define M_TOKEN_QUOTE           (1<<3)
/** flag for buffer_extract_token(): !)|~ are terms */
#define M_TOKEN_PATTERN         (1<<4)
/** flag for buffer_extract_token(): don't reap comments */
#define M_TOKEN_COMMENT         (1<<5)
/** flag for buffer_extract_token(): don't treat ; as special */
#define M_TOKEN_SEMICOLON       (1<<6)

/**
 * Expand special sequences of source buffer to destination buffer.
 *
 * The implementation is senstive to different quoting types. Special
 * sequences in "-quotes will be expanded while those in ' will just
 * be copied.
 *
 * @param dst Destination buffer.
 * @param token Source buffer.
 * @param flags Flags, see M_TOKEN_*.
 * @return Number of characters parsed from token.
 * @test buffer_tests::test_buffer_tokenize().
 */
size_t buffer_extract_token (buffer_t* dst, buffer_t* token, int flags);

/** @copydoc buffer_extract_token(). */
size_t buffer_extract_token2 (buffer_t* dst, const char* token, int flags);

#ifdef __cplusplus
}
#endif

#endif /* !MUTTNG_BUFFER_H */
