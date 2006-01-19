/**
 * @file core/net.c
 * @brief Implementation: Network-related routines
 *
 * This file is published under the GNU General Public License.
 */
#include "core_features.h"

#include "net.h"
#include "str.h"
#include "io.h"
#include "conv.h"
#include "mem.h"
#include "intl.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#ifdef CORE_LIBIDN
#include <idna.h>
#include <stringprep.h>
/* Work around incompatibilities in the libidn API */
# if (!defined(HAVE_IDNA_TO_ASCII_8Z) && defined(HAVE_IDNA_TO_ASCII_FROM_UTF8))
# define idna_to_ascii_8z(a,b,c) idna_to_ascii_from_utf8(a,b,(c)&1,((c)&2)?1:0)
# endif
# if (!defined(HAVE_IDNA_TO_ASCII_LZ) && defined(HAVE_IDNA_TO_ASCII_FROM_LOCALE))
# define idna_to_ascii_lz(a,b,c) idna_to_ascii_from_locale(a,b,(c)&1,((c)&2)?1:0)
# endif
# if (!defined(HAVE_IDNA_TO_UNICODE_8Z8Z) && defined(HAVE_IDNA_TO_UNICODE_UTF8_FROM_UTF8))
# define idna_to_unicode_8z8z(a,b,c) idna_to_unicode_utf8_from_utf8(a,b,(c)&1,((c)&2)?1:0)
# endif
#endif

/**
 * Remove trailing dot.
 * @param q String.
 */
static void strip_trailing_dot (char *q)
{
  char *p = q;

  for (; *q; q++)
    p = q;

  if (*p == '.')
    *p = '\0';
}

int net_dnsdomainname (buffer_t* dst) {
  FILE *f;
  char tmp[1024];
  char *p = NULL;
  char *q;

  if (!dst)
    return 0;

  if ((f = fopen ("/etc/resolv.conf", "r")) == NULL)
    return 0;

  tmp[sizeof (tmp) - 1] = 0;

  while (fgets (tmp, sizeof (tmp) - 1, f) != NULL) {
    p = tmp;
    while (ISSPACE (*p))
      p++;
    if (str_ncmp ("domain", p, 6) == 0
        || str_ncmp ("search", p, 6) == 0) {
      p += 6;

      for (q = strtok (p, " \t\n"); q; q = strtok (NULL, " \t\n"))
        if (strcmp (q, "."))
          break;

      if (q) {
        strip_trailing_dot (q);
        buffer_add_str(dst,q,-1);
        io_fclose (&f);
        return 1;
      }

    }
  }

  io_fclose (&f);
  return 0;
}

/**
 * See if string contains ASCII-only characters.
 * @param s String.
 * @return Yes/No.
 * @bug We really need to find a central place once for all those tiny
 * tool functions instead of adding them everywhere...
 */
static inline int is_ascii(const char* s) {
  register const char* p = s&&*s?s:"";
  while (*p) {
    if ((*p++ & 0x80))
      return 0;
  }
  return 1;
}

#ifndef CORE_LIBIDN
int net_idn2local (buffer_t* dst, buffer_t* src,
                       const char* local, unsigned short flags) {
  (void) local; (void) flags;
  if (!dst || !src) return 0;
  buffer_shrink(dst,0);
  buffer_add_buffer(dst,src);
  return 1;
}
int net_local2idn (buffer_t* dst, buffer_t* src, const char* local) {
  (void) local;
  if (!dst || !src) return 0;
  buffer_shrink(dst,0);
  buffer_add_buffer(dst,src);
  return is_ascii(dst->str);
}
int net_idn_version (buffer_t*dst) {
  (void) dst;
  return 0;
}
#else /* !CORE_LIBIDN */

int net_idn2local (buffer_t* dst, buffer_t* src,
                       const char* local, unsigned short flags) {

  if (!dst || !src || !local) return 0;

  buffer_free(dst);

  if (idna_to_unicode_8z8z(src->str,&dst->str,1)!=IDNA_SUCCESS)
    goto notrans;
  dst->len = str_len(dst->str);
  dst->size = dst->len+1;
  if (!conv_iconv(dst,"utf-8",local))
    goto notrans;

  if (!(flags & MI_MAY_BE_IRREVERSIBLE)) {
    int irrev = 0;
    buffer_t tmp;
    char* tmp2 = NULL;

    buffer_init(&tmp);
    buffer_add_buffer(&tmp,dst);

    if (!conv_iconv(&tmp,local,"utf-8"))
      irrev = 1;
    if (!irrev && idna_to_ascii_8z(tmp.str,&tmp2,1)!=IDNA_SUCCESS)
      irrev = 1;
    if (!irrev && buffer_equal1(&tmp,tmp2,-1))
      irrev = 1;
    mem_free(&tmp2);
    buffer_free(&tmp);
    if (irrev)
      goto notrans;
  }

  return 1;

notrans:
  buffer_shrink(dst,0);
  buffer_add_buffer(dst,src);
  return 0;
}

int net_local2idn (buffer_t* dst, buffer_t* src, const char* local) {
  short rc = 0;
  buffer_t tmp;

  if (!dst || !src || !local) return 0;

  buffer_free(dst);

  buffer_init(&tmp);
  buffer_add_buffer(&tmp,src);

  rc = conv_iconv(&tmp,local,"utf-8");
  if (rc && idna_to_ascii_8z(tmp.str,&dst->str,1)!=IDNA_SUCCESS) {
    buffer_shrink(dst,0);
    buffer_add_buffer(dst,src);
  }

  if (dst->str && *dst->str) {
    dst->len = str_len(dst->str);
    dst->size = dst->len+1;
  }

  buffer_free(&tmp);
  return rc && is_ascii(dst->str);
}

int net_idn_version (buffer_t* dst) {
  if (!dst) return 1;
  buffer_add_str(dst,"libidn ",7);
  buffer_add_str(dst,stringprep_check_version(NULL),-1);
  buffer_add_str(dst,_(" (compiled with "),-1);
  buffer_add_str(dst,STRINGPREP_VERSION,-1);
  buffer_add_str(dst,_(")"),-1);
  return 1;
}

#endif /* CORE_LIBIDN */
