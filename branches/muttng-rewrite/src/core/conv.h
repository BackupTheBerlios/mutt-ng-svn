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
 *   - 128 for bits
 *   - we may have a sign: +1
 *   - trailing \\0: +1
 */
#define NUMBUF  130

/**
 * Convert signed int to string.
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
 * @test buffer_tests::test_buffer_add_snum2().
 */
char* conv_itoa2 (char* buf, signed long num, short pad, short base);

/**
 * Convert signed int to string.
 * @b NOTE: when padding is requested, the number will have exactly the
 * number of digits given, i.e. padding does @b not mean to optionally
 * fill with leading zeros to at least get a given lenght but to always
 * exactly get the given length.
 * @param B Buffer of size NUMBUF.
 * @param N Number to convert.
 * @param P To how many digits to 0-pad the number. If negative,
 *          the number will be converted as-is.
 * @return Offset into buf.
 * @test buffer_tests::test_buffer_add_snum2().
 */
#define conv_itoa(B,N,P)        conv_itoa2(B,N,P,10)

/**
 * Convert unsigned int to string.
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
 * @test buffer_tests::test_buffer_add_unum2().
 */
char* conv_uitoa2 (char* buf, unsigned long num, short pad, short base);

/**
 * Convert unsigned int to string.
 * @b NOTE: when padding is requested, the number will have exactly the
 * number of digits given, i.e. padding does @b not mean to optionally
 * fill with leading zeros to at least get a given lenght but to always
 * exactly get the given length.
 * @param B Buffer of size NUMBUF.
 * @param N Number to convert.
 * @param P To how many digits to 0-pad the number. If negative,
 *          the number will be converted as-is.
 * @return Offset into buf.
 * @test buffer_tests::test_buffer_add_unum2().
 */
#define conv_uitoa(B,N,P)       conv_uitoa2(B,N,P,10)

/**
 * Initialize core's conversion part.
 * Use from core_init() only.
 */
void conv_init (void);

/**
 * Cleanup core's conversion part.
 * Use from core_cleanup() only.
 */
void conv_cleanup (void);

/**
 * Canonicalize a character set to sane MIME name.
 * @param charset Charset buffer to be fixed up as needed in-place.
 * @todo Make this publicly visible? We do want to abstract from things
 * like different character sets and different names for the same, etc,
 * right?
 * @bug Likely this is broken as it kicks any language spec defined by
 * RfC2231 which may be wrong in edge cases as this is done here
 * globally for the whole application.
 */
void conv_charset_normal (buffer_t* charset);

/**
 * List all supported character sets via callback. The listing is
 * aborted if the callback returns failure.
 * @param mime If true, list only MIME charsets but only aliases otherwise.
 * @param printfunc Callback to handle items.
 * @return Number of times callback was called.
 */
unsigned long conv_charset_list (int mime,
                                 int (*printfunc)(const char*));

/**
 * See if two character sets are identical.
 * These are first canonicalized if required.
 * @param c1 1st Character set.
 * @param c2 2nd Character set.
 */
int conv_charset_eq (const char* c1, const char* c2);

/**
 * See if transfer encoding is to be Base64 instead of one
 * automatically chosen.
 * @param charset Character set.
 * @return Force base64 or not.
 */
int conv_charset_base64 (const char* charset);

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
