#ifndef _LIB_INTL_H
#define _LIB_INTL_H

/*
 * config.h must be included by source file!
 */

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

#endif /* !_LIB_INTL_H */
