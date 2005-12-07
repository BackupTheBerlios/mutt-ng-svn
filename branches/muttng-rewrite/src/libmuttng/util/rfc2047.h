/** @ingroup libmuttng_util */
/**
 * @file libmuttng/util/rfc2047.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: RfC2047 encoding
 */
#ifndef LIBMUTTNG_UTIL_RFC2047_H
#define LIBMUTTNG_UTIL_RFC2047_H

#include "core/buffer.h"

bool rfc2047_encode (buffer_t* src, buffer_t* dst);
bool rfc2047_decode (buffer_t* src, buffer_t* dst);

#endif
