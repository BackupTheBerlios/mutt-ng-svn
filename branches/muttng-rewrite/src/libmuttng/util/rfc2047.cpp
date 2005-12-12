/** @ingroup libmuttng_util */
/**
 * @file libmuttng/util/rfc2047.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: RfC2047 encoding
 */
#include "libmuttng/util/rfc2047.h"
#include "libmuttng/libmuttng.h"

#include "core/conv.h"
#include "core/qp.h"
#include "core/base64.h"
#include "core/str.h"

#include <cstring>
#include <string.h>

/** temporary: debug on stdout? */
#define DO_DEBUG        0

#if DO_DEBUG
#include <iostream>
#endif

/** unsupported encoding */
#define ENC_INVALID     0
/** quoted-printable encoding */
#define ENC_QP          1
/** base64 encoding */
#define ENC_BASE64      2

/**
 * output character sets which attempt to use.
 * @bug make this configurable
 */
static const char* OutSets[] = { "us-ascii", "iso-8859-1", "iso-8859-15", "utf-8", NULL };

/**
 * hex chars for QP encoder
 */
static char Hex[] = "0123456789ABCDEF";

/** maximum length of <b>TOTAL</b> encoded word */
#define ENC_MAX_LEN     75
/** test given character for UTF-8 continuation bit */
#define ENC_CONT(X) (((X) & 0xc0) == 0x80)
/**
 * maximum length of <b>TEXT</b> in <b>UTF-8</b> encoded word.
 * Some math:
 *   -# maximum total length is: 75
 *   -# the prefix '=?utf-8?q?' and suffix '?=': 12
 *   -# when starting a multibyte character in UTF-8,
 *      assume we need 12 ascii chars for QP as worst case
 */
#define ENC_MAX_LEN2    (75-12-12)

/**
 * See if character is MIME-special.
 * @param c Character.
 * @return Yes/No.
 * @bug (almost) equal to special() in conv.c
 */
static inline bool is_special (unsigned char c) {
  switch (c) {
  case '(':
  case ')':
  case '<':
  case '>':
  case '@':
  case ',':
  case ';':
  case ':':
  case '"':
  case '/':
  case '[':
  case ']':
  case '?':
  case '.':
  case '=':
    return true;
  default:
    return false;
  }
}

/**
 * See if buffer needs RfC2047 encoding.
 * @param src Source string.
 * @return Yes/No.
 */
static inline bool need_enc (buffer_t* src) {
  register const char* p = src->str;
  while (*p && p-src->str < (int)src->len && *p >= 0x20 && *p <= 0x7e) p++;
  return *p!='\0';
}

/**
 * Quoted-printable encode a single character.
 * @param c Character to encode.
 * @param dst Destination storage.
 * @return Number of characters written. I.e. 1 or 3.
 */
static inline size_t encode_char_qp (unsigned char c, char dst[4]) {
  unsigned short f = 0;
  if (c <= 0x20 || c >= 0x7f || is_special(c)) {
    dst[f++] = '=';
    dst[f++] = Hex[(c&0xf0)>>4];
    dst[f++] = Hex[c&0x0f];
    dst[f] = '\0';
  } else {
    dst[f++] = c;
    dst[f] = '\0';
  }
  return (size_t)f;
}

/**
 * Encode a string in some character set as one encoded word.
 * This fails if the complete length would exceed the RfC2047-limit.
 * @param dst Destination buffer.
 * @param src Source buffer (fully encoded).
 * @param enc Which encoding is used.
 * @param in In which character set source is encoded.
 * @return
 *      -# @c true if source completely fits
 *      -# @c false otherwise
 */
static inline bool encode_as_one (buffer_t* dst, buffer_t* src,
                                  unsigned short enc, const char* in) {
  size_t inlen = str_len(in);
  if (ENC_MAX_LEN >= inlen+src->len+7) { /* 7: '=?'+'?X?'+'?=' */
    /* fine: buffer completely fits in one encoded word */
    buffer_add_str(dst,"=?",2);
    buffer_add_str(dst,in,inlen);
    buffer_add_str(dst,(enc==ENC_QP)?"?q?":"?b?",3);
    buffer_add_buffer(dst,src);
    buffer_add_str(dst,"?=",2);
    return true;
  }
  return false;
}

/**
 * Encode a string in UTF-8 to many encoded words.
 *
 * This is heavily broken and this is why: for RfC2047 we must encode
 * input to a self-contained encoded word. This especially means we must
 * intercept an encoded word within a multi-byte character. However, the
 * encoding may be different from the current locale so we would need a
 * method to detect complete multibyte sequences of an arbitrary
 * encoding. This would get as complicated as the old encoder is.
 *
 * Thus,
 * in the (IMHO for the majority of our users rare) case that a word
 * doesn't fit in we do this: we simply convert the string to UTF-8 for
 * which we have a trivial method to check for multibyte presence (see 
 * definition of ENC_CONT()) so we can easily QP-encode to multiple
 * words which are each shorter than RfC2047's limit.
 * @param dst Destination buffer.
 * @param src Source buffer.
 * @param in Current input character set. If it's not UTF-8, we convert
 *           to it by force and ignore any error we get since we at
 *           least must send something.
 * @bug This is heavily broken.
 */
static void encode_as_many (buffer_t* dst, buffer_t* src, const char* in) {
  char hex[4];
  size_t w = 0, l=0, i;

  if (!str_eq(in,"utf-8"))
    conv_iconv(src,in,"utf-8");

  for (i = 0; i < src->len; i++) {
    if (w == 0)
      buffer_add_str(dst,"=?utf-8?q?",10);
    l = encode_char_qp(src->str[i],hex);
    if ((w+l >= ENC_MAX_LEN2) && !ENC_CONT(src->str[i])) {
      if (i < src->len-1)
        buffer_add_str(dst,"?= ",3);
      buffer_add_str(dst,"=?utf-8?q?",10);
      w = 0;
    }
    buffer_add_str(dst,hex,l);
    w += l;
  }
  if (dst->str[dst->len-1] != '=')
    buffer_add_str(dst,"?=",2);
}

/**
 * Encode a single raw input word from initial portion of input.
 * @param dst Destination buffer.
 * @param src Source string.
 * @param len Length of initial portion in input.
 * @param in Character set input is encoded in.
 */
static void encode_word (buffer_t* dst, const unsigned char* src,
                         size_t len, const char* in) {
  unsigned short i;
  buffer_t text_conv, text_enc1, text_enc2;

  buffer_init(&text_conv);
  buffer_init(&text_enc1);
  buffer_init(&text_enc2);

  for (i = 0; OutSets[i]; i++) {
    buffer_shrink(&text_conv,0);
    buffer_add_str(&text_conv,(char*)src,len);
    /*
     * loop over all character sets we may use for sending and try
     * first one where input can be completely cnverted to
     */
    /* XXX */
    (void) SendCharset;
    if (conv_iconv(&text_conv,in,OutSets[i]) > 0) {
#if DO_DEBUG
      std::cout<<"will encode='"<<(NONULL(text_conv.str))<<"' (l="<<len<<")"<<std::endl;
#endif
      /*
       * can convert from input to current output charset cleanly;
       * now see if QP or Base64 is shorter
       */
      buffer_shrink(&text_enc1,0); buffer_shrink(&text_enc2,0);
      buffer_encode_qp(&text_enc1,&text_conv,'=');
      buffer_encode_base64(&text_enc2,&text_conv);
      bool qp = text_enc1.len<text_enc2.len;
      /*
       * try to encode as a single word; if it doesn't fit,
       * do multiple and we're done
       */
      if (!encode_as_one(dst,qp?&text_enc1:&text_enc2,qp?ENC_QP:ENC_BASE64,OutSets[i]))
        encode_as_many(dst,&text_conv,OutSets[i]);
      break;
    }
  }

  buffer_free(&text_conv);
  buffer_free(&text_enc1);
  buffer_free(&text_enc2);
}

bool rfc2047_encode (buffer_t* dst, buffer_t* src, const char* in) {
  if (!dst || !src)
    return false;

  unsigned char* p, *last;
  bool wasenc = false;
  p = last = (unsigned char*)src->str;
  buffer_shrink(dst,0);

  if (!need_enc(src))
    buffer_add_buffer(dst,src);
  else {
    while (*p) {
      if (*p < 0x20 || *p > 0x7e) {
        /* p==non-ASCII character: encode word */
        register unsigned char* start = p, *end = p;
        /* seach backwards for WS to at most start of input */
        while(*start && start > (unsigned char*)src->str && *start != ' ') start--;
        /* search forward for WS to at most end of input */
        while(*end && (end-(unsigned char*)src->str) < (int)src->len && *end != ' ') end++;
        if (*start == ' ') start++;
        /*
         * add clean string between encoded words;
         * buffer_add_str() on its own handles start-last==0
         */
        buffer_add_str(dst,(char*)last,start-last);
        encode_word(dst,(unsigned char*)start,end-start,in);
        /* if last word we added to dst was encoded in previous loop
         * * but we have no clean word added in the current loop, add 1 space
         * * between the two encoded words
         */
        if (start-last == 0 && wasenc)
          buffer_add_ch(dst,' ');
        wasenc = true;
        p = end;
        last = end;
      } else
        p++;
    }
    /* add last ASCII word (if any) */
    buffer_add_str(dst,(char*)last,p-last);
  }

#if DO_DEBUG
  std::cout<<"orig='"<<(NONULL(src->str))<<"'"<<std::endl;
  std::cout<<"encoded='"<<(NONULL(dst->str))<<"'"<<std::endl;
#endif

  return true;
}

/**
 * See if character marks a valid encoding.
 * @param c Character.
 * @return Encoding type.
 */
static inline unsigned short is_encoding (unsigned char c) {
  switch (c) {
  case 'B':
  case 'b':
    return ENC_BASE64;
  case 'Q':
  case 'q':
    return ENC_QP;
  default:
    return ENC_INVALID;
  }
}

/**
 * Find the start and end of the first encoded word in the string.
 * We use the grammar in section 2 of RfC2047, but the "encoding"
 * must be [bB] or [qQ]. Also, we don't require the encoded word to be
 * separated by linear-white-space (section 5(1)).
 * @param str Source string.
 * @param endptr Pointer set to the first character after encoded
 *               word found for contination elsewhere.
 * @return start pointer of encoded word or @c NULL if not found.
 */
static const char *find_encoded_word (const char *str, const char **endptr) {
  const char *p, *q;

  q = str;
  while ((p = strstr (q, "=?"))) {
    /* forward in charset to find end */
    for (q = p + 2; 0x20 < *q && *q < 0x7f && !is_special(*q); q++);
    /* see if encoding afterwards is valid */
    if (q[0] != '?' || is_encoding(q[1])==ENC_INVALID || q[2] != '?')
      continue;
    /* forward in text to find end */
    for (q = q + 3; 0x20 <= *q && *q < 0x7f && *q != '?'; q++);
    if (q[0] != '?' || q[1] != '=') {
      --q;
      continue;
    }
    
    *endptr = q + 2;
    return p;
  }
  return NULL;
}

/**
 * RfC2047-decode a single word to given output character set.
 * @param dst Destination buffer for storing result.
 * @param src Source string. This is expected to be the result of
 *            find_encoded_word().
 * @param out Output character set.
 */
static bool decode_word (buffer_t* dst, const char* src, const char* out) {
  unsigned short count = 0, enc = 0;
  const char* work, *qmark;
  buffer_t charset, text_enc, text_dec;

  buffer_init(&charset);
  buffer_init(&text_enc);
  buffer_init(&text_dec);

  /*
   * format:
   *    =?[charset]?[encoding]?[text]?=
   * when splitting by question marks, fields are (count below):
   *    1. '='
   *    2. [charset]
   *    3. [encoding]
   *    4. [text]
   *    5. '='
   * find_encoded_word() ensured we have =? and ?= so we need
   * to check for fields ∈ [2,4] only
   */
  for (work = src; (qmark = strchr(work,'?')); work = qmark+1) {
    switch (++count) {
    case 2:
      /* get charset */
      buffer_add_str(&charset,work,qmark-work);
#if DO_DEBUG
      std::cout<<"charset='"<<charset.str<<"'"<<std::endl;
#endif
      break;
    case 3:
      /* get encoding */
      enc = is_encoding((unsigned char)*work);
#if DO_DEBUG
      std::cout<<"encoding="<<enc<<std::endl;
#endif
      break;
    case 4:
      /* get text */
      buffer_add_str(&text_enc,work,qmark-work);
#if DO_DEBUG
      std::cout<<"text_enc='"<<text_enc.str<<"'"<<std::endl;
#endif
      /* decode */
      switch(enc) {
      case ENC_QP:      if (!buffer_decode_qp(&text_dec,&text_enc,'=',NULL)) goto bail; break;
      case ENC_BASE64:  if (!buffer_decode_base64(&text_dec,&text_enc,NULL)) goto bail; break;
      case ENC_INVALID:
      default:          goto bail; break;
      }
      /* convert (ignoring result) and copy */
#if DO_DEBUG
      std::cout<<"text_dec='"<<text_dec.str<<"'"<<std::endl;
#endif
      conv_iconv(&text_dec,charset.str,out);
      buffer_add_buffer(dst,&text_dec);
      break;
    default:
      break;
    }
  }

  buffer_free(&charset);
  buffer_free(&text_enc);
  buffer_free(&text_dec);
  return true;

bail:
#if DO_DEBUG
  std::cout<<"decode failed"<<std::endl;
#endif
  /* in case of incorrectly encoded words, just reconstruct original as-is */
  buffer_add_str(dst,"=?",2);
  buffer_add_buffer(dst,&charset);
  buffer_add_ch(dst,'?');
  buffer_add_buffer(dst,&text_enc);
  buffer_add_str(dst,"?=",2);

  buffer_free(&charset);
  buffer_free(&text_enc);
  buffer_free(&text_dec);
  return false;
}

/**
 * Add cleaned text between two encoded words.
 * Though space between encoded words may include linebreaks and header
 * folding, we expect unfolding to be done in a Header class already
 * so we do this:
 *   -# if all @c len chars from @c src match <tt>[ \\t]</tt>, we add nothing to @c dst
 *   -# otherwise add @c len chars from @c src as-is
 * @param dst Destination buffer.
 * @param src Source string.
 * @param len Length of source string to check for spaces.
 */
static inline void add_clean (buffer_t* dst, const char* src, size_t len) {
  size_t i = 0;
  if (len == 0)
    return;
  while (i < len) {
    if (src[i] != ' ' && src[i] != '\t') {
      buffer_add_str(dst,src,len);
      return;
    }
    i++;
  }
}

bool rfc2047_decode (buffer_t* dst, buffer_t* src, const char* out) {
  if (!src)
    return false;

  const char* start,    /* current start of encoded word */
              *end,     /* end of current word */
              *last;    /* end of last word to get text between 2 words */
  bool local = false, rc = true;
  buffer_t* test = dst;

  /* as storage for dst param is optional, we work on local buffer */
  if (!test) {
    local = true;
    test = new buffer_t;
    buffer_init(test);
  }

  last = end = src->str;
  buffer_shrink(test,0);

  while (end-src->str < (int) src->len) {
    /* from current position, get start of encoded word */
    if (!(start = find_encoded_word(end,&end))) {
      /*
       * no encoded words: just convert to dst charset
       * XXX see if we really need to convert here
       */
      buffer_add_buffer(test,src);
      rc = conv_iconv(test,"us-ascii",out);
      break;
    }
    /* add text between, (try to) decode, continue */
    add_clean(test,last,start-last);
    if (!decode_word(test,start,out)) {
      rc = false;
      break;
    }
    last = end;
  }

  if (local) {
    buffer_free(test);
    delete test;
  }

  return rc;
}
