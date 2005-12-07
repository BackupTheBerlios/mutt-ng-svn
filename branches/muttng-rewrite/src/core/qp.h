#ifndef CORE_QP_H
#define CORE_QP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "buffer.h"

/**
 * Decode a QP string.
 * This will fail on incomplete and the magic @c %00 sequence.
 * @param str String to decode.
 * @param c Magic QP delimiter character. This is @c % for URLs
 *          and mostly @c = in all other cases.
 * @param chars If given a pointer to @c int, this stores how many
 *              characters of input have been processed to point
 *              out at which place an error occured.
 * @return Success.
 */
int qp_decode (char* str, unsigned char c, int* chars);

void qp_encode (buffer_t* src, buffer_t* dst);

#ifdef __cplusplus
}
#endif

#endif
