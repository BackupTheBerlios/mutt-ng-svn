/** @ingroup libmuttng_util */
/**
 * @file libmuttng/util/recode.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: Encoding/Decoding routines
 */
#include <ctype.h>
#include <cstring>

#include "libmuttng/util/recode.h"

/**
 * Interpret character as hex and return decimal value.
 * @param c Character.
 * @return
 *   - decimal value upon success
 *   - @c -1 upon error
 */
static int hexval (unsigned char c) {
  if (c >= '0' && c <= '9')
    return (c - '0');
  unsigned char d = tolower (c);
  if (d >= 'a' && d <= 'f')
    return (10 + (d - 'a'));
  return (-1);
}

void recode_encode_qp (buffer_t* buf) { (void) buf; }

bool recode_decode_qp (char* str, unsigned char c, int* chars) {
  char* d;

  if (!str)
    return (true);

  if (chars)
    *chars = 0;

  for (d = str; *str; str++) {
    if (*str == c) {
      /**
       * Be sure to block %00 aka \\0, there were some bugtraq
       * posts so be strict ;-)
       */
      if (str[1] && str[2] && strncmp (&str[1], "00", 2) != 0 &&
          hexval ((unsigned char) str[1]) >= 0 &&
          hexval ((unsigned char) str[2]) >= 0) {
        *d++ = (hexval ((unsigned char) str[1]) << 4) |
               (hexval ((unsigned char) str[2]));
        str += 2;
      } else
        return (false);
    }
#if 0
    else if (*str == '+')
      *d++ = ' ';
#endif
    else
      *d++ = *str;
    if (chars)
      (*chars)++;
  }
  *d = '\0';
  return (true);
}
