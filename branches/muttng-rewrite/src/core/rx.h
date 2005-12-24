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

#include <regex.h>

#include "buffer.h"

/** regular expression abstraction */
typedef struct rx_t {
  /** printable version */
  char *pattern;
  /** compiled expression */
  regex_t *rx;
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
 */
rx_t* rx_compile (const char* pattern, buffer_t* error, int flags);

/**
 * Free memory for rx_t structure.
 * @b NOTE: This does not free the structure itself.
 * @param rx rx_t structure.
 */
void rx_free (rx_t* rx);

/**
 * Execute search.
 * @param rx Previously compiled rx_t.
 * @param str String to match.
 * @bug find abstract match description
 */
int rx_exec (rx_t* rx, const char* str);

/**
 * Compare two rx_t for equality.
 * @param rx1 1st regex.
 * @param rx2 2nd regex.
 * @return 1 if equal, 0 otherwise.
 */
int rx_eq (const rx_t* rx1, const rx_t* rx2);

#ifdef __cplusplus
}
#endif

#endif /* !_LIB_RX_H */
