/** @ingroup core_string */
/**
 * @file core/conv.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: Conversion routines
 */
#ifndef MUTTNG_CORE_CONV_H
#define MUTTNG_CORE_CONV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "buffer.h"

/**
 * Buffer size for converting int to string via conv_itoa().
 * As we may convert to binary:
 *   - we need 8*sizeof(int) bits, i.e. character cells
 *   - we may have a sign: +1
 *   - trailing \\0: +1
 */
#define NUMBUF  ((sizeof(int)*8)+2)

/**
 * Convert int to string.
 * @b NOTE: when padding is requested, the number will have exactly the
 * number of digits given, i.e. padding does @b not mean to optionally
 * fill with leading zeros to at least get a given lenght but to always
 * exactly get the given length.
 * @param buf Buffer of size NUMBUF.
 * @param num Number to convert.
 * @param pad To how many digits to 0-pad the number. If negative,
 *            the number will be converted as-is.
 * @param base Number base (2, 8, 10, 16).
 * @return Offset into buf.
 * @test buffer_tests::test_buffer_add_num2().
 */
char* conv_itoa2 (char* buf, int num, int pad, int base);

/**
 * Convert int to string.
 * @b NOTE: when padding is requested, the number will have exactly the
 * number of digits given, i.e. padding does @b not mean to optionally
 * fill with leading zeros to at least get a given lenght but to always
 * exactly get the given length.
 * @param B Buffer of size NUMBUF.
 * @param N Number to convert.
 * @param P To how many digits to 0-pad the number. If negative,
 *          the number will be converted as-is.
 * @return Offset into buf.
 * @test buffer_tests::test_buffer_add_num2().
 */
#define conv_itoa(B,N,P)        conv_itoa2(B,N,P,10)

/**
 * See if two character sets are identical.
 * These are first canonicalized if required.
 * @param c1 1st Character set.
 * @param c2 2nd Character set.
 */
int conv_charset_eq (const char* c1, const char* c2);

/**
 * Convert a buffer using @c iconv().
 * If no libiconv support was built in, this does nothing at all.
 * @param src Source string to be converted in-place.
 * @param in Input character set.
 * @param out Output character set.
 * @return Whether conversion succeeded or not.
 * @test conv_tests::test_iconv().
 */
int conv_iconv (buffer_t* src, const char* in, const char* out);

/**
 * Get iconv() version.
 * @param dst Optional destination buffer.
 * @return Wether compiled with iconv support or not.
 */
int conv_iconv_version (buffer_t* dst);

#ifdef __cplusplus
}
#endif

#endif /* !MUTTNG_CORE_CONV_H */
