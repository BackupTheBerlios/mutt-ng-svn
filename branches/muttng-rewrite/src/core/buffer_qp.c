/** @ingroup core_conv */
/**
 * @file core/buffer_qp.c
 * @brief Implementation: Quoted-printable conversions
 */
#include <ctype.h>
#include <string.h>

#include "buffer.h"

/** alphabet */
static const char Hex[] = { "0123456789ABCDEF" };

/**
 * Interpret character as hex and return decimal value.
 * @param c Character.
 * @return
 *   - decimal value upon success
 *   - @c -1 upon error
 */
static inline int dec (unsigned char c) {
  if (c >= '0' && c <= '9')
    return (c - '0');
  if (c >= 'a' && c <= 'f')
    return (10 + (c - 'a'));
  if (c >= 'A' && c <= 'F')
    return (10 + (c - 'A'));
  return (-1);
}

/**
 * See if character is special enough for MIME to be encoded.
 * @todo This is not really optimal to have here but this way there's
 * only one decoder for QP at all instead of duplicate once or passing
 * around function pointers for special character checking.
 * @param c Character.
 * @return Yes/No.
 */
static inline int special (unsigned char c) {
  switch (c) {
  case '@':
  case '.':
  case ',':
  case ':':
  case '<':
  case '>':
  case '[':
  case ']':
  case '\\':
  case '"':
  case '(':
  case ')':
  case '?':
  case '/':
  case '\t':
  /* for RfC2047, this is allowed for space instead of =20 so encode */
  case '_':
    return 1;
  default:
    return 0;
  }
}

void buffer_qp_encode(buffer_t* dst, const buffer_t* src, unsigned char c) {
  int i = 0;
  if (!src || !dst)
    return;
  buffer_shrink(dst,0);
  for (i = 0; i < (int)src->len; i++) {
    unsigned char e = src->str[i];
    /* catch space here instead of in specials() */
    if (e <= 0x20 || e > 0x7e || e == c || special(e)) {
      buffer_add_ch(dst,c);
      buffer_add_ch(dst,Hex[(e&0xf0)>>4]);
      buffer_add_ch(dst,Hex[e&0x0f]);
    } else
      buffer_add_ch(dst,e);
  }
}

int buffer_qp_decode(buffer_t* dst, const buffer_t* src, unsigned char c, size_t* chars) {
  int i = 0;

  if (!src || !dst)
    return 0;

  buffer_shrink(dst,0);

  for (i = 0; i < (int)src->len; i++) {
    if ((unsigned char)src->str[i] == c) {
      /**
       * Be sure to block %00 aka \\0, there were some bugtraq
       * posts so be strict ;-)
       */
      if (i < ((int) src->len-2) &&
          ((unsigned char) src->str[i+1] != '0' ||
           (unsigned char) src->str[i+2] != '0') &&
          dec ((unsigned char) src->str[i+1]) >= 0 &&
          dec ((unsigned char) src->str[i+2]) >= 0) {
        buffer_add_ch(dst,(dec ((unsigned char) src->str[i+1]) << 4) |
                          (dec ((unsigned char) src->str[i+2])));
        i+=2;
      } else {
        if (chars)
          *chars = (size_t) i;
        return (0);
      }
    }
    else
      buffer_add_ch(dst,(unsigned char)src->str[i]);
  }
  if (chars)
    *chars = 0;
  return (1);
}
