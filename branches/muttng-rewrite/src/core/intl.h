/**
 * @ingroup core
 * @addtogroup Internationalization
 * @{
 */
/**
 * @file core/intl.h
 * @brief Interface: Internationalization
 */
#ifndef MUTTNG_CORE_INTL_H
#define MUTTNG_CORE_INTL_H

/**
 * @def _(X)
 * Translate string using current locale.
 * @param X string.
 * @return Translated one or original if not found.
 */

/**
 * @def N_(X)
 * Translate string using current locale. Use for constant initialization.
 * @param X string.
 * @return Translated one or original if not found.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "core_features.h"

# ifdef CORE_INTL
#  include <libintl.h>
# define _(X) (gettext (X))
#  ifdef gettext_noop
#   define N_(X) gettext_noop (X)
#  else
#   define N_(X) (X)
#  endif
# else
#  define _(X) (X)
#  define N_(X) X
# endif

/**
 * @def intl_init().
 * Initialize internationalization.
 */

#ifdef CORE_INTL
void intl_init(void);
#else
#define intl_init
#endif

/**
 * @def intl_encoding(charset).
 * Set encoding of translated messages.
 * @param charset Character encoding to use.
 */
#ifdef CORE_INTL
void intl_encoding(const char* charset);
#else
#define intl_encoding(X)
#endif

#ifdef __cplusplus
}
#endif

#endif /* !MUTTNG_CORE_INTL_H */

/** @} */
