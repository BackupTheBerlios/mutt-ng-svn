/*
 * Copyright notice from original mutt:
 * Copyright (C) 1999-2000 Thomas Roessler <roessler@does-not-exist.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#ifndef _CHARSET_H
#define _CHARSET_H

#ifdef HAVE_ICONV_H
#include <iconv.h>
#endif

#ifndef HAVE_ICONV_T_DEF
typedef void *iconv_t;
#endif

#ifndef HAVE_ICONV
#define ICONV_CONST /**/
  iconv_t iconv_open (const char *, const char *);
size_t iconv (iconv_t, ICONV_CONST char **, size_t *, char **, size_t *);
int iconv_close (iconv_t);
#endif

int mutt_convert_string (char **, const char *, const char *, int);
char *mutt_get_first_charset (const char *);
int mutt_convert_nonmime_string (char **);

iconv_t mutt_iconv_open (const char *, const char *, int);
size_t mutt_iconv (iconv_t, ICONV_CONST char **, size_t *, char **, size_t *,
                   ICONV_CONST char **, const char *);

typedef void *FGETCONV;

FGETCONV *fgetconv_open (FILE *, const char *, const char *, int);
int fgetconv (FGETCONV *);
char *fgetconvs (char *, size_t, FGETCONV *);
void fgetconv_close (FGETCONV **);

void mutt_set_langinfo_charset (void);

#define M_ICONV_HOOK_FROM 1
#define M_ICONV_HOOK_TO   2

#endif /* _CHARSET_H */
