/*
 * Copyright notice from original mutt:
 * Copyright (C) 2003 Thomas Roessler <roessler@does-not-exist.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#ifndef _MUTT_IDNA_H
# define _MUTT_IDNA_H

#include "rfc822.h"
#include "charset.h"

#ifdef HAVE_LIBIDN
#include <idna.h>
#endif

#define MI_MAY_BE_IRREVERSIBLE		(1 << 0)

int mutt_idna_to_local (const char *, char **, int);
int mutt_local_to_idna (const char *, char **);

int mutt_addrlist_to_idna (ADDRESS *, char **);
int mutt_addrlist_to_local (ADDRESS *);

void mutt_env_to_local (ENVELOPE *);
int mutt_env_to_idna (ENVELOPE *, char **, char **);

const char *mutt_addr_for_display (ADDRESS * a);

/* Work around incompatibilities in the libidn API */

#ifdef HAVE_LIBIDN
# if (!defined(HAVE_IDNA_TO_ASCII_8Z) && defined(HAVE_IDNA_TO_ASCII_FROM_UTF8))
#  define idna_to_ascii_8z(a,b,c) idna_to_ascii_from_utf8(a,b,(c)&1,((c)&2)?1:0)
# endif
# if (!defined(HAVE_IDNA_TO_ASCII_LZ) && defined(HAVE_IDNA_TO_ASCII_FROM_LOCALE))
#  define idna_to_ascii_lz(a,b,c) idna_to_ascii_from_locale(a,b,(c)&1,((c)&2)?1:0)
# endif
# if (!defined(HAVE_IDNA_TO_UNICODE_8Z8Z) && defined(HAVE_IDNA_TO_UNICODE_UTF8_FROM_UTF8))
#  define idna_to_unicode_8z8z(a,b,c) idna_to_unicode_utf8_from_utf8(a,b,(c)&1,((c)&2)?1:0)
# endif
#endif

#endif
