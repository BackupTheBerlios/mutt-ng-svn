/**
 * @ingroup core
 */
/**
 * @file core/conv.c
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Conversion routines implementation
 */
#include <stdlib.h>

#include "conv.h"

static const char* Alph = "0123456789abcdef";

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
