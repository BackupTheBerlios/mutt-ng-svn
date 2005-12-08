/** @ingroup core_conv */
/**
 * @file core/qp.c
 * @brief Implementation: Quoted-printable conversions
 */
#include <ctype.h>
#include <string.h>
#include "qp.h"

/** alphabet */
static const char Hex[] = { "0123456789ABCDEF" };

/**
 * Interpret character as hex and return decimal value.
 * @param c Character.
 * @return
 *   - decimal value upon success
 *   - @c -1 upon error
 */
static int hexval (unsigned char c) {
  unsigned char d;
  if (c >= '0' && c <= '9')
    return (c - '0');
  d = tolower (c);
  if (d >= 'a' && d <= 'f')
    return (10 + (d - 'a'));
  return (-1);
}

void qp_encode (buffer_t* dst, const buffer_t* src, unsigned char c) {
  int i = 0;
  if (!src || !dst)
    return;
  buffer_shrink(dst,0);
  for (i = 0; i < (int)src->len; i++) {
    if (!isalnum((unsigned char)src->str[i]) ||
        (unsigned char)src->str[i] == c) {
      buffer_add_ch(dst,c);
      buffer_add_ch(dst,Hex[((unsigned char)src->str[i]) / 16]);
      buffer_add_ch(dst,Hex[((unsigned char)src->str[i]) % 16]);
    } else
      buffer_add_ch(dst,(unsigned char)src->str[i]);
  }
}

int qp_decode (buffer_t* dst, const buffer_t* src, unsigned char c, size_t* chars) {
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
      if (i < ((int) src->len-2) && strncmp (&src->str[i+1], "00", 2) != 0 &&
          hexval ((unsigned char) src->str[i+1]) >= 0 &&
          hexval ((unsigned char) src->str[i+2]) >= 0) {
        buffer_add_ch(dst,(hexval ((unsigned char) src->str[i+1]) << 4) |
                          (hexval ((unsigned char) src->str[i+2])));
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
