/**
 * @ingroup core
 */
/**
 * @file core/intl.h
 * @brief Internationalization interface
 * @b NOTE: config.h must be included by source file!
 */
#ifndef MUTTNG_CORE_INTL_H
#define MUTTNG_CORE_INTL_H

#ifdef __cplusplus
extern "C" {
#endif

# ifdef ENABLE_NLS
#  include <libintl.h>
# define _(a) (gettext (a))
#  ifdef gettext_noop
#   define N_(a) gettext_noop (a)
#  else
#   define N_(a) (a)
#  endif
# else
#  define _(a) (a)
#  define N_(a) a
# endif

#ifdef __cplusplus
}
#endif

#endif /* !MUTTNG_CORE_INTL_H */
