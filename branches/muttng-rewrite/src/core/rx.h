/**
 * @ingroup core
 * @addtogroup core_rx Regular Expressions
 * @{
 */
/**
 * @file core/rx.h
 * @brief Interface: Regular expressions
 */
#ifndef CORE_RX_H
#define CORE_RX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "buffer.h"

/** abstract representation of a match */
typedef struct rx_match_t {
  /** start offset into subject string */
  size_t start;
  /** first char after end of match */
  size_t end;
} rx_match_t;

/** regular expression abstraction */
typedef struct rx_t {
  /** printable version */
  char *pattern;
  /** compiled expression */
  void* rx;
  /** do not match */
  int no;
} rx_t;

/** ignore case */
#define RX_NO_CASE      (1<<0)
/** treat \\n specially */
#define RX_NEWLINE      (1<<1)

/**
 * Compile a pattern to a regular expressions.
 * @param pattern Pattern.
 * @param error Optional buffer for error message.
 * @param flags Flags. See RX_*.
 * @return rx_t pointer or @c NULL in case of error.
 * @test rx_tests::test_rx_compile().
 */
rx_t* rx_compile (const char* pattern, buffer_t* error, int flags);

/**
 * Free memory for rx_t structure.
 * @b NOTE: This does not free the structure itself.
 * @param rx rx_t structure.
 * @test rx_tests::test_rx_compile().
 */
void rx_free (rx_t* rx);

/**
 * Only see if string matches, don't obtain any details.
 * @param rx Previously compiled rx_t.
 * @param str String to match.
 * @test rx_tests::test_rx_match().
 */
int rx_match (rx_t* rx, const char* str);

/**
 * See if string matches and obtain any details.
 * @param rx Previously compiled rx_t.
 * @param str String to match.
 * @param matches Storage for matches.
 * @param matchcnt How many matches to obtain.
 */
int rx_exec (rx_t* rx, const char* str, rx_match_t* matches, size_t matchcnt);

/**
 * Compare two rx_t for equality.
 * @param rx1 1st regex.
 * @param rx2 2nd regex.
 * @return 1 if equal, 0 otherwise.
 */
int rx_eq (const rx_t* rx1, const rx_t* rx2);

/**
 * Get regular expression backend version.
 * @param dst Optional destination buffer.
 * @return Wether compiled with pcre support or not.
 */
int rx_version (buffer_t* dst);

#ifdef __cplusplus
}
#endif

#endif /* !_LIB_RX_H */
