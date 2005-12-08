/** @ingroup libmuttng_util */
/**
 * @file libmuttng/util/rfc2047.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: RfC2047 encoding
 */
#ifndef LIBMUTTNG_UTIL_RFC2047_H
#define LIBMUTTNG_UTIL_RFC2047_H

#include "core/buffer.h"

/**
 * Encode a string according to RfC 2047.
 * @param dst Destination buffer.
 * @param src Source string.
 * @return Success.
 */
bool rfc2047_encode (buffer_t* dst, buffer_t* src);

/**
 * Decode a string according to RfC 2047.
 * @param dst Destination buffer.
 * @param src Source string.
 * @return Success.
 */
bool rfc2047_decode (buffer_t* dst, buffer_t* src);

#endif
