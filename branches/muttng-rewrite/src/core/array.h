/**
 * @ingroup core
 */
/**
 * @file core/array.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Array handling
 */
#ifndef MUTTNG_CORE_ARRAY_H
#define MUTTNG_CORE_ARRAY_H

/**
 * Allocate memory for bit array.
 * @param n Number of bits.
 * @return n Bits allocated.
 */
#define array_bit_alloc(n) calloc ((n + 7) / 8, sizeof (char))

/**
 * Set bit in bit array.
 * @param v Array
 * @param n Bit number
 */
#define array_bit_set(v,n) v[n/8] |= (1 << (n % 8))

/**
 * Unset bit in bit array.
 * @param v Array
 * @param n Bit number
 */
#define array_bit_unset(v,n) v[n/8] &= ~(1 << (n % 8))

/**
 * Toggle bit in bit array.
 * @param v Array
 * @param n Bit number
 */
#define array_bit_toggle(v,n) v[n/8] ^= (1 << (n % 8))

/**
 * Query bit in bit array.
 * @param v Array
 * @param n Bit number
 * @return Set or unset.
 */
#define array_bit_isset(v,n) (v[n/8] & (1 << (n % 8)))

#endif /* !MUTTNG_CORE_ARRAY_H */

/** @} */
