/** @ingroup libmuttng_util */
/**
 * @file libmuttng/util/rfc2047.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: RfC2047 encoding
 *
 * This file is published under the GNU General Public License.
 */
#ifndef LIBMUTTNG_UTIL_RFC2047_H
#define LIBMUTTNG_UTIL_RFC2047_H

#include "core/buffer.h"

#include "libmuttng/libmuttng.h"

/**
 * RfC2047-compliant encoder/decoder class
 */
class RfC2047 : public LibMuttng {
  public:
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
  private:
    /**
     * Encode a single raw input word from initial portion of input.
     * @param dst Destination buffer.
     * @param src Source string.
     * @param len Length of initial portion in input.
     * @param in Character set input is encoded in.
     */
    void encode_word (buffer_t* dst, const unsigned char* src,
                      size_t len, const char* in);
    /**
     * RfC2047-decode a single word to given output character set.
     * @param dst Destination buffer for storing result.
     * @param src Source string. This is expected to be the result of
     *            find_encoded_word().
     * @param out Output character set.
     */
    bool decode_word (buffer_t* dst, const char* src, const char* out);

};

#endif
