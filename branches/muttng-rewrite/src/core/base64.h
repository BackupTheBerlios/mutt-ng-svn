/** @ingroup core_conv */
/**
 * @file core/base64.h
 * @brief Interface: Base64 conversions
 */
#ifndef CORE_BASE64__H
#define CORE_BASE64__H

#include "buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Decodes base64-encoded buffer.
 * @param dest buffer into which the decoded string shall be written.
 * @param src buffer which shall be decoded.
 * @param chars If given a pointer to @c size_t, this stores how many
 *              characters of input have been processed to point
 *              out at which place an error occured.
 * @return Success.
 */
int buffer_decode_base64(buffer_t * dest, const buffer_t * src, size_t* chars);

/**
 * Encodes a buffer to base64.
 * @param dest buffer into which the encoded string shall be written.
 * @param src buffer which shall be encoded.
 */
void buffer_encode_base64(buffer_t * dest, const buffer_t * src);

#ifdef __cplusplus
}
#endif

#endif /* !CORE_BASE64__H */
