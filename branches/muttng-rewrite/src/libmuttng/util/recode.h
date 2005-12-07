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

/**
 * Encode a string to QP.
 * @param buf String.
 */
void recode_encode_qp (buffer_t* buf);

/**
 * Decode a string from QP.
 * @param buf String.
 * @return Success.
 */
int recode_decode_qp (buffer_t* buf);

/**
 * Encode a string to base64.
 * @param buf String.
 */
void recode_encode_base64 (buffer_t* buf);

/**
 * Decode a string from base64.
 * @param buf String.
 * @return Success.
 */
int recode_decode_base64 (buffer_t* buf);

#endif
