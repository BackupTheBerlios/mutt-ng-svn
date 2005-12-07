/** @ingroup libmuttng_util */
/**
 * @file libmuttng/util/rfc2047.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: RfC2047 encoding
 */
#ifndef LIBMUTTNG_UTIL_RFC2047_H
#define LIBMUTTNG_UTIL_RFC2047_H

#include "core/buffer.h"
#include "libmuttng/util/recode.h"

/**
 * RfC2047-encode a string.
 * @param buf String to encode.
 * @param method Encoding method.
 * @return Success.
 */
int rfc2047_encode (buffer_t* buf, encodings method);

/**
 * Decode a string from QP.
 * @param buf String to decode.
 * @param method Which method to use.
 * @return Success.
 */
int rfc2047_decode (buffer_t* buf, encodings method);

#endif
