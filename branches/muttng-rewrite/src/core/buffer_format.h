/** @ingroup core_string */
/**
 * @file core/buffer_format.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: String buffer
 *
 * This file is published under the GNU Lesser General Public License.
 */
#ifndef MUTTNG_CORE_BUFFER_FORMAT_H
#define MUTTNG_CORE_BUFFER_FORMAT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "buffer_base.h"

/**
 * printf()-like format a buffer.
 * @param dst Destination buffer.
 * @param src Source buffer.
 * @param callback Callback to use for expansion.
 *                 For the @c fmt parameter, the buffer just contains
 *                 offsets into the source and correct length info
 *                 (i.e. no memory copy), so please please please make
 *                 use of the length info.
 * @return
 *   - -1 in case of error
 *   - number of times callback was called
 * @test buffer_tests::test_buffer_format().
 */
int buffer_format (buffer_t* dst, buffer_t* src,
                   int callback(buffer_t* dst, buffer_t* fmt, unsigned char c));

#ifdef __cplusplus
}
#endif

#endif /* !MUTTNG_BUFFER_H */
