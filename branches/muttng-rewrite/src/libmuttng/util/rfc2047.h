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
 * @param in Input character set, ie in which one source is encoded.
 * @return Success.
 */
bool rfc2047_encode (buffer_t* dst, buffer_t* src, const char* in);

/**
 * Decode a string according to RfC 2047.
 * @param dst Optional Destination buffer. If none given, just test
 *            whether decoding would work.
 * @param src Source string.
 * @param out Output charecter set, ie to which one to decode.
 * @return Success.
 * @test rfc2047_tests::test_decode().
 * @test rfc2047_tests::test_decode_buf().
 */
bool rfc2047_decode (buffer_t* dst, buffer_t* src, const char* out);

#endif
