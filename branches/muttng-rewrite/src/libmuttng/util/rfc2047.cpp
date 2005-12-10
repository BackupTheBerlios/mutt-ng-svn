/** @ingroup libmuttng_util */
/**
 * @file libmuttng/util/rfc2047.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: RfC2047 encoding
 */
#include "libmuttng/util/rfc2047.h"

#include "core/conv.h"
#include "core/qp.h"
#include "core/base64.h"
#include "core/str.h"

#include <cstring>

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

bool rfc2047_encode (buffer_t* dst, buffer_t* src) {
  (void) src; (void) dst;
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
 *   -# if all @c len chars from @c src match <tt>[ \t]</tt>, we add nothing to @c dst
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
