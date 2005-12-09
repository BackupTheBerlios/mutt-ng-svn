/** @ingroup core_string */
/**
 * @file core/conv.c
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: Conversion routines
 */
#include "core_features.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#ifdef CORE_LIBICONV
#include <iconv.h>
#endif

#include "conv.h"
#include "str.h"
#include "mem.h"

/** alphabet of numbers */
static const char* Alph = "0123456789abcdef";

#ifdef CORE_LIBICONV
/**
 * Map strange charset names to MIME ones.
 * The following list has been created manually from the data under:
 * http://www.iana.org/assignments/character-sets
 * Last update: 2000-09-07
 *
 * Note that it includes only the subset of character sets for which
 * a preferred MIME name is given.
 */
static struct {
  /** what we may see in the wild */
  const char *key;
  /** what we want to use instead */
  const char *pref;
} PreferredMIMENames[] = {
  {
  "ansi_x3.4-1968", "us-ascii"}, {
  "iso-ir-6", "us-ascii"}, {
  "iso_646.irv:1991", "us-ascii"}, {
  "ascii", "us-ascii"}, {
  "iso646-us", "us-ascii"}, {
  "us", "us-ascii"}, {
  "ibm367", "us-ascii"}, {
  "cp367", "us-ascii"}, {
  "csASCII", "us-ascii"}, {
  "csISO2022KR", "iso-2022-kr"}, {
  "csEUCKR", "euc-kr"}, {
  "csISO2022JP", "iso-2022-jp"}, {
  "csISO2022JP2", "iso-2022-jp-2"}, {
  "ISO_8859-1:1987", "iso-8859-1"}, {
  "iso-ir-100", "iso-8859-1"}, {
  "iso_8859-1", "iso-8859-1"}, {
  "latin1", "iso-8859-1"}, {
  "l1", "iso-8859-1"}, {
  "IBM819", "iso-8859-1"}, {
  "CP819", "iso-8859-1"}, {
  "csISOLatin1", "iso-8859-1"}, {
  "ISO_8859-2:1987", "iso-8859-2"}, {
  "iso-ir-101", "iso-8859-2"}, {
  "iso_8859-2", "iso-8859-2"}, {
  "latin2", "iso-8859-2"}, {
  "l2", "iso-8859-2"}, {
  "csISOLatin2", "iso-8859-2"}, {
  "ISO_8859-3:1988", "iso-8859-3"}, {
  "iso-ir-109", "iso-8859-3"}, {
  "ISO_8859-3", "iso-8859-3"}, {
  "latin3", "iso-8859-3"}, {
  "l3", "iso-8859-3"}, {
  "csISOLatin3", "iso-8859-3"}, {
  "ISO_8859-4:1988", "iso-8859-4"}, {
  "iso-ir-110", "iso-8859-4"}, {
  "ISO_8859-4", "iso-8859-4"}, {
  "latin4", "iso-8859-4"}, {
  "l4", "iso-8859-4"}, {
  "csISOLatin4", "iso-8859-4"}, {
  "ISO_8859-6:1987", "iso-8859-6"}, {
  "iso-ir-127", "iso-8859-6"}, {
  "iso_8859-6", "iso-8859-6"}, {
  "ECMA-114", "iso-8859-6"}, {
  "ASMO-708", "iso-8859-6"}, {
  "arabic", "iso-8859-6"}, {
  "csISOLatinArabic", "iso-8859-6"}, {
  "ISO_8859-7:1987", "iso-8859-7"}, {
  "iso-ir-126", "iso-8859-7"}, {
  "ISO_8859-7", "iso-8859-7"}, {
  "ELOT_928", "iso-8859-7"}, {
  "ECMA-118", "iso-8859-7"}, {
  "greek", "iso-8859-7"}, {
  "greek8", "iso-8859-7"}, {
  "csISOLatinGreek", "iso-8859-7"}, {
  "ISO_8859-8:1988", "iso-8859-8"}, {
  "iso-ir-138", "iso-8859-8"}, {
  "ISO_8859-8", "iso-8859-8"}, {
  "hebrew", "iso-8859-8"}, {
  "csISOLatinHebrew", "iso-8859-8"}, {
  "ISO_8859-5:1988", "iso-8859-5"}, {
  "iso-ir-144", "iso-8859-5"}, {
  "ISO_8859-5", "iso-8859-5"}, {
  "cyrillic", "iso-8859-5"}, {
  "csISOLatinCyrillic", "iso8859-5"}, {
  "ISO_8859-9:1989", "iso-8859-9"}, {
  "iso-ir-148", "iso-8859-9"}, {
  "ISO_8859-9", "iso-8859-9"}, {
  "latin5", "iso-8859-9"},      /* this is not a bug */
  {
  "l5", "iso-8859-9"}, {
  "csISOLatin5", "iso-8859-9"}, {
  "ISO_8859-10:1992", "iso-8859-10"}, {
  "iso-ir-157", "iso-8859-10"}, {
  "latin6", "iso-8859-10"},     /* this is not a bug */
  {
  "l6", "iso-8859-10" }, {
  "csISOLatin6", "iso-8859-10" }, {
  "csKOI8r", "koi8-r"}, {
  "MS_Kanji", "Shift_JIS"},     /* Note the underscore! */
  {
  "csShiftJis", "Shift_JIS"}, {
  "Extended_UNIX_Code_Packed_Format_for_Japanese", "EUC-JP"}, {
  "csEUCPkdFmtJapanese", "EUC-JP"}, {
  "csGB2312", "gb2312"}, {
  "csbig5", "big5"},
    /* 
     * End of official brain damage.  What follows has been taken
     * from glibc's localedata files. 
     */
  {
  "iso_8859-13", "iso-8859-13"}, {
  "iso-ir-179", "iso-8859-13"}, {
  "latin7", "iso-8859-13"},     /* this is not a bug */
  {
  "l7", "iso-8859-13"}, {
  "iso_8859-14", "iso-8859-14"}, {
  "latin8", "iso-8859-14"},     /* this is not a bug */
  {
  "l8", "iso-8859-14"}, {
  "iso_8859-15", "iso-8859-15"}, {
  "latin9", "iso-8859-15"},     /* this is not a bug */
    /* Suggested by Ionel Mugurel Ciobica <tgakic@sg10.chem.tue.nl> */
  {
  "latin0", "iso-8859-15"},     /* this is not a bug */
  {
  "iso_8859-16", "iso-8859-16"}, {
  "latin10", "iso-8859-16"},    /* this is not a bug */
    /* 
     * David Champion <dgc@uchicago.edu> has observed this with
     * nl_langinfo under SunOS 5.8. 
     */
  {
  "646", "us-ascii"},
    /* 
     * http://www.sun.com/software/white-papers/wp-unicode/
     */
  {
  "eucJP", "euc-jp"}, {
  "PCK", "Shift_JIS"}, {
  "ko_KR-euc", "euc-kr"}, {
  "zh_TW-big5", "big5"},
    /* seems to be common on some systems */
  {
  "sjis", "Shift_JIS"}, {
  "euc-jp-ms", "eucJP-ms"},
    /*
     * If you happen to encounter system-specific brain-damage with
     * respect to character set naming, please add it above this
     * comment, and submit a patch to <mutt-dev@mutt.org>. 
     */
    /* End of aliases.  Please keep this line last. */
  {
  NULL, NULL}
};
#endif /* !CORE_LIBICONV */

char* conv_itoa2 (char* buf, int num, int pad, int base) {
  int i = NUMBUF-2, p = pad, sign = num < 0;

  buf[NUMBUF-1] = '\0';
  switch (base) {
    case 2:
    case 8:
    case 10:
    case 16:
      break;
    default:
      base = 10;
      break;
  }
  if (num == 0 && pad < 0) {
    buf[i--] = '0';
    return (buf+i+1);
  }
  if (pad < 0)
    pad = i;
  while ((num != 0 || p > 0) && i >= sign && pad > 0) {
    buf[i--] = Alph[abs (num % base)];
    num /= base;
    pad--;
  }
  if (sign)
    buf[i--] = '-';
  return (buf+i+1);
}

#ifndef CORE_LIBICONV
int conv_iconv (buffer_t* src, const char* in, const char* out) {
  (void) src; (void) in; (void) out;
  return 1;
}
int conv_iconv_version (buffer_t* dst) {
  (void) dst;
  return 0;
}
#else

int conv_iconv_version (buffer_t* dst) {
  int v =
#ifdef _LIBICONV_VERSION
    _LIBICONV_VERSION;
#else
    0;
#endif
  if (!dst)
    return 1;
  buffer_add_str(dst,"libiconv ",9);
  buffer_add_num(dst,v>>8,-1);
  buffer_add_ch(dst,'.');
  buffer_add_num(dst,v&0xff,-1);
  return 1;
}

/**
 * Canonicalize a character set to sane MIME name.
 * @param charset Charset buffer to be fixed up as needed in-place.
 * @todo Make this publicly visible? We do want to abstract from things
 * like different character sets and different names for the same, etc,
 * right?
 */
static void charset_canonical (buffer_t* charset) {
  size_t i;
  char *p;

  /** @bug catch some common iso-8859-something misspellings */
#if 0
  if (!strncasecmp (charset->str, "8859", 4) && charset->str[4] != '-')
    snprintf (scratch, sizeof (scratch), "iso-8859-%s", name + 4);
  else if (!ascii_strncasecmp (name, "8859-", 5))
    snprintf (scratch, sizeof (scratch), "iso-8859-%s", name + 5);
  else if (!ascii_strncasecmp (name, "iso8859", 7) && name[7] != '-')
    snprintf (scratch, sizeof (scratch), "iso_8859-%s", name + 7);
  else if (!ascii_strncasecmp (name, "iso8859-", 8))
    snprintf (scratch, sizeof (scratch), "iso_8859-%s", name + 8);
  else
    strfcpy (scratch, NONULL (name), sizeof (scratch));
#endif

  for (i = 0; PreferredMIMENames[i].key; i++)
    if (strcasecmp(charset->str,PreferredMIMENames[i].key) == 0) {
      buffer_shrink(charset,0);
      buffer_add_str(charset,PreferredMIMENames[i].pref,-1);
      return;
    }

  /* for cosmetics' sake, transform to lowercase. */
  for (p = charset->str; *p; p++)
    *p = tolower (*p);
}

static iconv_t my_iconv_open (const char *tocode, const char *fromcode) {
  buffer_t tocode1, fromcode1;
  iconv_t cd;

  buffer_init(&tocode1);
  buffer_add_str(&tocode1,tocode,-1);

  buffer_init(&fromcode1);
  buffer_add_str(&fromcode1,fromcode,-1);

  charset_canonical(&tocode1);
  charset_canonical(&fromcode1);

  cd = iconv_open(tocode1.str,fromcode1.str);

  buffer_free(&tocode1);
  buffer_free(&fromcode1);

  return (cd);
}

/*
 * Like iconv, but keeps going even when the input is invalid
 * If you're supplying inrepls, the source charset should be stateless;
 * if you're supplying an outrepl, the target charset should be.
 */

static size_t my_iconv (iconv_t cd, CORE_ICONV_CONST char **inbuf, size_t * inbytesleft,
                        char **outbuf, size_t * outbytesleft,
                        CORE_ICONV_CONST char **inrepls, const char *outrepl) {
  size_t ret = 0, ret1;
  CORE_ICONV_CONST char *ib = *inbuf;
  size_t ibl = *inbytesleft;
  char *ob = *outbuf;
  size_t obl = *outbytesleft;

  for (;;) {
    ret1 = iconv (cd, &ib, &ibl, &ob, &obl);
    if (ret1 != (size_t) - 1)
      ret += ret1;
    if (ibl && obl && errno == EILSEQ) {
      if (inrepls) {
        /* Try replacing the input */
        CORE_ICONV_CONST char **t;

        for (t = inrepls; *t; t++) {
          CORE_ICONV_CONST char *ib1 = *t;
          size_t ibl1 = str_len (*t);
          char *ob1 = ob;
          size_t obl1 = obl;

          iconv (cd, &ib1, &ibl1, &ob1, &obl1);
          if (!ibl1) {
            ++ib, --ibl;
            ob = ob1, obl = obl1;
            ++ret;
            break;
          }
        }
        if (*t)
          continue;
      }
      /* Replace the output */
      if (!outrepl)
        outrepl = "?";
      iconv (cd, 0, 0, &ob, &obl);
      if (obl) {
        int n = str_len (outrepl);

        if (n > (int) obl) {
          outrepl = "?";
          n = 1;
        }
        memcpy (ob, outrepl, n);
        ++ib, --ibl;
        ob += n, obl -= n;
        ++ret;
        iconv (cd, 0, 0, 0, 0); /* for good measure */
        continue;
      }
    }
    *inbuf = ib, *inbytesleft = ibl;
    *outbuf = ob, *outbytesleft = obl;
    return ret;
  }
}

int conv_charset_eq (const char* c1, const char* c2) {
  int rc = 0;
  buffer_t ch1, ch2;

  buffer_init(&ch1);
  buffer_add_str(&ch1,c1,-1);

  buffer_init(&ch2);
  buffer_add_str(&ch2,c2,-1);

  charset_canonical(&ch1);
  charset_canonical(&ch2);

  rc = str_eq(ch1.str,ch2.str);

  buffer_free(&ch1);
  buffer_free(&ch2);
  return rc;
}

/*
 * Convert a string
 * Used in rfc2047.c and rfc2231.c
 */

int conv_iconv (buffer_t* src, const char* from, const char* to) {
  iconv_t cd;
  CORE_ICONV_CONST char *repls[] = { "\357\277\275", "?", 0 };
  char* s = NULL;

  if (!src)
    return 0;

  /* don't attempt to do anything upon equality */
  if (to && from && conv_charset_eq(to,from))
    return 1;

  s = str_dup (src->str);

  if (s && to && from && (cd = my_iconv_open (to, from)) != (iconv_t) - 1) {
    int len;
    CORE_ICONV_CONST char *ib;
    char *buf, *ob;
    size_t ibl, obl;
    CORE_ICONV_CONST char **inrepls = 0;
    char *outrepl = 0;

    if (conv_charset_eq(to,"utf-8"))
      outrepl = "\357\277\275";
    else if (conv_charset_eq(from,"utf-8"))
      inrepls = repls;
    else
      outrepl = "?";

    len = str_len (s);
    ib = s, ibl = len + 1;
    obl = MB_LEN_MAX * ibl;
    ob = buf = mem_malloc (obl + 1);

    my_iconv (cd, &ib, &ibl, &ob, &obl, inrepls, outrepl);
    iconv_close (cd);

    *ob = '\0';

    buffer_shrink(src,0);
    buffer_add_str(src,buf,-1);
    mem_free(&s);
    mem_free(&buf);
    return 1;
  }
  mem_free(&s);
  return 0;
}

#endif /* CORE_LIBICONV */
