/*
 * Copyright notice from original mutt:
 * [none]
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#ifndef _MBYTE_H
# define _MBYTE_H

#include "config.h"

# ifdef HAVE_WC_FUNCS
#  ifdef HAVE_WCHAR_H
#   include <wchar.h>
#  endif
#  ifdef HAVE_WCTYPE_H
#   include <wctype.h>
#  endif
# endif

#ifndef HAVE_WC_FUNCS
size_t wcrtomb (char *s, wchar_t wc, mbstate_t * ps);
size_t mbrtowc (wchar_t * pwc, const char *s, size_t n, mbstate_t * ps);
int iswprint (wint_t wc);
int iswspace (wint_t wc);
int iswalnum (wint_t wc);
wint_t towupper (wint_t wc);
wint_t towlower (wint_t wc);
int wcwidth (wchar_t wc);
#endif /* !HAVE_WC_FUNCS */


void mutt_set_charset (char *charset);
extern int Charset_is_utf8;
size_t utf8rtowc (wchar_t * pwc, const char *s, size_t n, mbstate_t * _ps);
wchar_t replacement_char (void);

#endif /* _MBYTE_H */
