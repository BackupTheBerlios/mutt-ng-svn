/** @ingroup libmuttng_util */
/**
 * @file libmuttng/util/recode.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: Encoding/Decoding routines
 */
#ifndef LIBMUTTNG_UTIL_RECODE_H
#define LIBMUTTNG_UTIL_RECODE_H

#include "core/buffer.h"

/** encoding routines we know */
typedef enum {
  /** @c quoted-printable */
  ENC_QP = 0,
  /** @c base64 */
  ENC_BASE64
} encodings;

void recode_encode_qp (buffer_t* buf);

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
bool recode_decode_qp (char* str, unsigned char c, int* chars);

#endif
