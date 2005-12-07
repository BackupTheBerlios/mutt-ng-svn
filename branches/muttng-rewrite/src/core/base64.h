#ifndef LIBMUTTNG_UTIL_BASE64__H
#define LIBMUTTNG_UTIL_BASE64__H

#include "buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Decodes base64-encoded buffer.
 * @param dest buffer into which the decoded string shall be written.
 * @param src buffer which shall be decoded.
 * @return length of decoded string.
 */
unsigned long buffer_decode_base64(buffer_t * dest, const buffer_t * src);

/**
 * Encodes a buffer to base64.
 * @param dest buffer into which the encoded string shall be written.
 * @param src buffer which shall be encoded.
 * @return length of encoded string.
 */
unsigned long buffer_encode_base64(buffer_t * dest, const buffer_t * src);

#ifdef __cplusplus
}
#endif


#endif
