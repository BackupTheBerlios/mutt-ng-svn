/**
 * @ingroup core
 */
/**
 * @file core/conv.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Conversion routines interface
 */
#ifndef MUTTNG_CORE_CONV_H
#define MUTTNG_CORE_CONV_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Buffer size for converting int to string via conv_itoa().
 * As we may convert to binary: sign + 32 bits + \0 = 34.
 */
#define NUMBUF  34

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
 */
char* conv_itoa2 (char* buf, int num, int pad, int base);

#define conv_itoa(B,N,P)        conv_itoa2(B,N,P,10)

#ifdef __cplusplus
}
#endif

#endif /* !MUTTNG_CORE_CONV_H */
