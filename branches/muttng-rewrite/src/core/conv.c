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
#include <errno.h>

#ifdef CORE_LIBICONV
#include <iconv.h>
#endif

#include "conv.h"
#include "str.h"
#include "mem.h"
#include "hash.h"

#ifdef CORE_LIBICONV

/** Hash table of known character sets for faster access */
static void* Charsets = NULL;

/**
 * Map strange charset names to MIME ones.
 * The following list has been created manually from the data under:
 * http://www.iana.org/assignments/character-sets
 *
 * Last update: 2005-12-19
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

  { "ansi_x3.4-1968", "us-ascii" },
  { "iso-ir-6", "us-ascii" }, 
  { "iso_646.irv:1991", "us-ascii"},
  { "ascii", "us-ascii"}, 
  { "iso646-us", "us-ascii"}, 
  { "us", "us-ascii"}, 
  { "ibm367", "us-ascii"}, 
  { "cp367", "us-ascii"}, 
  { "csascii", "us-ascii"}, 

  { "csiso2022kr", "iso-2022-kr"}, 
  { "cseuckr", "euc-kr"}, 
  { "csiso2022jp", "iso-2022-jp"}, 

  { "csiso2022jp2", "iso-2022-jp-2"}, 

  { "iso_8859-1:1987", "iso-8859-1"},
  { "iso-ir-100", "iso-8859-1"},
  { "iso_8859-1", "iso-8859-1"},
  { "latin1", "iso-8859-1"},
  { "l1", "iso-8859-1"},
  { "ibm819", "iso-8859-1"},
  { "cp819", "iso-8859-1"},
  { "csisolatin1", "iso-8859-1"},

  { "iso_8859-2:1987", "iso-8859-2"},
  { "iso-ir-101", "iso-8859-2"},
  { "iso_8859-2", "iso-8859-2"},
  { "latin2", "iso-8859-2"},
  { "l2", "iso-8859-2"},
  { "csisolatin2", "iso-8859-2"},

  { "iso_8859-3:1988", "iso-8859-3"},
  { "iso-ir-109", "iso-8859-3"},
  { "iso_8859-3", "iso-8859-3"},
  { "latin3", "iso-8859-3"},
  { "l3", "iso-8859-3"},
  { "csisolatin3", "iso-8859-3"},

  { "iso_8859-4:1988", "iso-8859-4"},
  { "iso-ir-110", "iso-8859-4"},
  { "iso_8859-4", "iso-8859-4"},
  { "latin4", "iso-8859-4"},
  { "l4", "iso-8859-4"},
  { "csisolatin4", "iso-8859-4"},

  { "iso_8859-6:1987", "iso-8859-6"},
  { "iso-ir-127", "iso-8859-6"},
  { "iso_8859-6", "iso-8859-6"},
  { "ecma-114", "iso-8859-6"},
  { "asmo-708", "iso-8859-6"},
  { "arabic", "iso-8859-6"},
  { "csisolatinarabic", "iso-8859-6"},

  { "iso_8859-7:1987", "iso-8859-7"},
  { "iso-ir-126", "iso-8859-7"},
  { "iso_8859-7", "iso-8859-7"},
  { "elot_928", "iso-8859-7"},
  { "ecma-118", "iso-8859-7"},
  { "greek", "iso-8859-7"},
  { "greek8", "iso-8859-7"},
  { "csisolatingreek", "iso-8859-7"},

  { "iso_8859-8:1988", "iso-8859-8"},
  { "iso-ir-138", "iso-8859-8"},
  { "iso_8859-8", "iso-8859-8"},
  { "hebrew", "iso-8859-8"},
  { "csisolatinhebrew", "iso-8859-8"},

  { "iso_8859-5:1988", "iso-8859-5"},
  { "iso-ir-144", "iso-8859-5"},
  { "iso_8859-5", "iso-8859-5"},
  { "cyrillic", "iso-8859-5"},
  { "csisolatincyrillic", "iso8859-5"},

  { "iso_8859-9:1989", "iso-8859-9"},
  { "iso-ir-148", "iso-8859-9"},
  { "iso_8859-9", "iso-8859-9"},
  { "latin5", "iso-8859-9"},      /* this is not a bug */
  { "l5", "iso-8859-9"},
  { "csisolatin5", "iso-8859-9"},

  { "iso_8859-10:1992", "iso-8859-10"},
  { "iso-ir-157", "iso-8859-10"},
  { "latin6", "iso-8859-10"},     /* this is not a bug */
  { "l6", "iso-8859-10" },
  { "csisolatin6", "iso-8859-10" },

  { "cskoi8r", "koi8-r"},

  { "ms_kanji", "shift_jis"},     /* note the underscore! */
  { "csshiftjis", "shift_jis"},

  { "extended_unix_code_packed_format_for_japanese", "euc-jp"},
  { "cseucpkdfmtjapanese", "euc-jp"},

  { "csgb2312", "gb2312"},

  { "csbig5", "big5"},

  /* 
   * end of official brain damage.  what follows has been taken
   * from glibc's localedata files. 
   */
  { "iso_8859-13", "iso-8859-13"},
  { "iso-ir-179", "iso-8859-13"},
  { "latin7", "iso-8859-13"},     /* this is not a bug */
  { "l7", "iso-8859-13"},

  { "iso_8859-14", "iso-8859-14"},
  { "latin8", "iso-8859-14"},     /* this is not a bug */
  { "l8", "iso-8859-14"},

  { "iso_8859-15", "iso-8859-15"},
  { "latin9", "iso-8859-15"},     /* this is not a bug */
  /* suggested by ionel mugurel ciobica <tgakic@sg10.chem.tue.nl> */
  { "latin0", "iso-8859-15"},     /* this is not a bug */

  { "iso_8859-16", "iso-8859-16"},
  { "latin10", "iso-8859-16"},    /* this is not a bug */

  /* 
   * david champion <dgc@uchicago.edu> has observed this with
   * nl_langinfo under sunos 5.8. 
   */
  { "646", "us-ascii"},
  /* 
   * http://www.sun.com/software/white-papers/wp-unicode/
   */
  { "eucjp", "euc-jp"},

  { "pck", "shift_jis"},
  { "sjis", "shift_jis"},

  { "ko_kr-euc", "euc-kr"},

  { "zh_tw-big5", "big5"},

  /* seems to be common on some systems */
  { "euc-jp-ms", "eucjp-ms"},

  { "iso_8859-6-e",             "iso-8859-6-e" },
  { "csiso88596e",              "iso-8859-6-e" },

  { "iso_8859-6-i",             "iso-8859-6-i" },
  { "csiso88596i",              "iso-8859-6-i" },

  { "iso_8859-8-e",             "iso-8859-8-e" },
  { "csiso88598e",              "iso-8859-8-e" },

  { "iso_8859-8-i",             "iso-8859-8-i" },
  { "csiso88598i",              "iso-8859-8-i" },

  /*
   * if you happen to encounter system-specific brain-damage with
   * respect to character set naming, please add it above this
   * comment, and submit a patch to <muttng-devel@lists.berlios.de>.
   */

  /* End of aliases.  Please keep this line last. */
  {
  NULL, NULL}
};
#endif /* !CORE_LIBICONV */

#ifndef CORE_LIBICONV
void conv_init () {}
void conv_cleanup () {}
void conv_charset_normal (buffer_t* charset) {
  (void) charset;
}
int conv_charset_eq (const char* s1, const char* s2) {
  return strcmp (s1,s2)==0;
}
unsigned long conv_charset_list (int mime,
                       int (*printfunc)(const char*)) {
  (void)mime;
  (void)printfunc;
  return 0;
}
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
  buffer_add_snum(dst,v>>8,-1);
  buffer_add_ch(dst,'.');
  buffer_add_snum(dst,v&0xff,-1);
  return 1;
}

/**
 * Canonicalize a character set to sane MIME name.
 * @param charset Charset buffer to be fixed up as needed in-place.
 * @todo Make this publicly visible? We do want to abstract from things
 * like different character sets and different names for the same, etc,
 * right?
 * @bug Likely this is broken as it kicks any language spec defined by
 * RfC2231 which may be wrong in edge cases as this is done here
 * globally for the whole application.
 */
void conv_charset_normal (buffer_t* charset) {
  size_t i;
  unsigned int code;
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

  /* for cosmetics' sake, transform to lowercase. */
  for (p = charset->str; *p; p++)
    *p = tolower (*p);

  code = hash_key(charset->str);
  if (hash_exists_hash(Charsets,charset->str,code) && 
      ((i = hash_find_hash(Charsets,charset->str,code)) != ((HASH_ITEMTYPE)-1))) {
    buffer_shrink(charset,0);
    buffer_add_str(charset,PreferredMIMENames[i].pref,-1);
    return;
  }

  /*
   * RfC2231 defines language spec syntax for charsets:
   *    [charset]*[langspec]
   * As we don't support it elsewhere, just kick this part for all
   */
  if ((p = strchr(charset->str,'*')))
    buffer_shrink(charset,p-charset->str);
}

void conv_init () {
  int i;
  Charsets = hash_new(2*sizeof(PreferredMIMENames)/(2*sizeof(char*)),0);
  for (i=0; PreferredMIMENames[i].key; i++) {
    hash_add(Charsets,PreferredMIMENames[i].key,(HASH_ITEMTYPE)i);
    hash_add(Charsets,PreferredMIMENames[i].pref,(HASH_ITEMTYPE)-1);
  }
}

void conv_cleanup () {
  hash_destroy(&Charsets,NULL);
}

/**
 * Callback for hash_map(): get all MIME charset names.
 * @param key Charset.
 * @param idx Index into PreferredMIMENames[].
 * @param moredata Typecasted printfunc or conv_charset_list().
 * @return 1.
 */
static int list_mime (const char* key, HASH_ITEMTYPE idx,
                       unsigned long moredata) {
  int (*printfunc)(const char*) = ((int(*)(const char*)) moredata);
  if (idx == ((HASH_ITEMTYPE)-1)) 
    printfunc(key);
  return 1;
}

/**
 * Callback for hash_map(): get all charset aliases.
 * @param key Charset.
 * @param idx Index into PreferredMIMENames[].
 * @param moredata Typecasted printfunc or conv_charset_list().
 * @return 1.
 */
static int list_alias (const char* key, HASH_ITEMTYPE idx,
                        unsigned long moredata) {
  int (*printfunc)(const char*) = ((int(*)(const char*)) moredata);
  (void)idx;
  printfunc(key);
  return 1;
}

#ifdef iconvlist
/**
 * Add character set from foreign source, ie libiconvlist().
 * @param count Size of names array.
 * @param names Names.
 * @param data Destination hash table.
 * @return 0
 */
static int add_foreign(unsigned int count, const char* const* names, void* data) {
  unsigned int i;
  char* key;
  for (i=0; i<count; i++) {
    key = str_dup(names[i]);
    str_tolower(key);
    hash_add(data,key,0);
    mem_free(&key);
  }
  return 0;
}
#endif

/**
 * Callback for hash_map(): Add character set from out source, ie PreferredMIMENames[].
 * @param key Key.
 * @param data Index into array.
 * @param moredata Destination hash table.
 * @return 1
 */
static int add_our(const char* key, HASH_ITEMTYPE data,
                   unsigned long moredata) {
  void* table = (void*)moredata;
  hash_add(table,key,data);
  return 1;
}

unsigned long conv_charset_list (int mime, int (*printfunc)(const char*)) {
  unsigned long rc;
  if (mime)
    /* if we do MIME, this is easy */
    rc = hash_map(Charsets,1,list_mime,(unsigned long)printfunc);
  else {
    /*
     * if we don't do MIME: create temporary table with our and add those
     * from libiconv (so we have sorting over complete set)
     */
    void* tmp = hash_new(hash_fill(Charsets),1);
    hash_map(Charsets,0,add_our,(unsigned long)tmp);
#ifdef iconvlist
    iconvlist(add_foreign,tmp);
#endif
    rc = hash_map(tmp,1,list_alias,(unsigned long)printfunc);
    hash_destroy(&tmp,NULL);
  }
  return rc;
}

/**
 * Like original @c iconv_open() but canoninitialized character sets
 * first.
 * @param tocode To which charset to convert.
 * @param fromcode From which charset to convert.
 * @return Conversion description from @c iconv_open().
 */
static iconv_t my_iconv_open (const char *tocode, const char *fromcode) {
  buffer_t tocode1, fromcode1;
  iconv_t cd;

  buffer_init(&tocode1);
  buffer_add_str(&tocode1,tocode,-1);

  buffer_init(&fromcode1);
  buffer_add_str(&fromcode1,fromcode,-1);

  conv_charset_normal(&tocode1);
  conv_charset_normal(&fromcode1);

  cd = iconv_open(tocode1.str,fromcode1.str);

  buffer_free(&tocode1);
  buffer_free(&fromcode1);

  return (cd);
}

/**
 * Like iconv, but keeps going even when the input is invalid
 * If you're supplying inrepls, the source charset should be stateless;
 * if you're supplying an outrepl, the target charset should be.
 * @param cd Conversion description.
 * @param inbuf Input string.
 * @param inbytesleft How many input bytes to convert.
 * @param outbuf Destination string.
 * @param outbytesleft How many output bytes to convert.
 * @param inrepls Replacements for bad characters in input.
 * @param outrepl Replacements for bad characters in output.
 * @param err Pointer to storage for whether an error occured.
 * @return Return value returned by @c iconv() or 0 in case of error.
 */
static size_t my_iconv (iconv_t cd, CORE_ICONV_CONST char **inbuf, size_t * inbytesleft,
                        char **outbuf, size_t * outbytesleft,
                        CORE_ICONV_CONST char **inrepls, const char *outrepl,
                        int* err) {
  size_t ret = 0, ret1;
  CORE_ICONV_CONST char *ib = *inbuf;
  size_t ibl = *inbytesleft;
  char *ob = *outbuf;
  size_t obl = *outbytesleft;

  *err = 0;

  for (;;) {
    ret1 = iconv (cd, &ib, &ibl, &ob, &obl);
    if (ret1 != (size_t) - 1)
      ret += ret1;
    else
      *err = 1;
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

  conv_charset_normal(&ch1);
  conv_charset_normal(&ch2);

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
  int err = 0;

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

    my_iconv (cd, &ib, &ibl, &ob, &obl, inrepls, outrepl, &err);
    iconv_close (cd);

    *ob = '\0';

    buffer_shrink(src,0);
    buffer_add_str(src,buf,-1);
    mem_free(&s);
    mem_free(&buf);
    return err==0;
  }
  mem_free(&s);
  return 0;
}

#endif /* CORE_LIBICONV */

int conv_charset_base64 (const char* charset) {
  buffer_t tmp;
  int rc = 0;

  if (!charset || !*charset)
    return rc;

  buffer_init(&tmp);
  buffer_add_str(&tmp,charset,-1);
  conv_charset_normal(&tmp);

  /*
   * RfC 1468: use base64 for iso-2022-jp
   * check for length exactly as RfCs also define
   * iso-2022-jp-[12] for which we can choose encoding
   */
  if (buffer_equal1(&tmp,"iso-2022-jp",11))
    rc = 1;

  buffer_free(&tmp);
  return rc;
}
