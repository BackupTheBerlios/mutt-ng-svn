#include <ctype.h>
#include <string.h>
#include "qp.h"

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

void qp_encode (buffer_t* src, buffer_t* dst) {(void)src;(void)dst;}

int qp_decode (char* str, unsigned char c, int* chars) {
  char* d;

  if (!str)
    return (1);

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
        return (0);
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
  return (1);
}
