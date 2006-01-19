/**
 * @ingroup core
 * @addtogroup core_conv Data conversion
 * @{
 */
/**
 * @file core/buffer_qp.h
 * @brief Inteface: Quoted-printable conversions
 */
#ifndef CORE_BUFFER_QP_H
#define CORE_BUFFER_QP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "buffer.h"

/**
 * Decode a QP string.
 * This will fail on incomplete and the magic @c %00 sequence.
 * @param dst Destination buffer.
 * @param src Source buffer to decode.
 * @param c Magic QP character. This is @c % for URLs
 *          and mostly @c = in all other cases.
 * @param chars If given a pointer to @c size_t, this stores how many
 *              characters of input have been processed to point
 *              out at which place an error occured.
 * @return Success.
 */
int buffer_qp_decode(buffer_t* dst, const buffer_t* src, unsigned char c, size_t* chars);

/**
 * Encode string to QP.
 * @param dst Destination buffer.
 * @param src Source string to encode.
 * @param c Magic QP character. This is @c % for URLs
 *          and mostly @c = in all other cases.
 */
void buffer_qp_encode(buffer_t* dst, const buffer_t* src, unsigned char c);

#ifdef __cplusplus
}
#endif

#endif /* !CORE_QP_H */

/** @} */
