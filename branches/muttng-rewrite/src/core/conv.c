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

char* conv_itoa (char* buf, int num, int pad) {
  int i = NUMBUF-2, p = pad, sign = num < 0;

  buf[NUMBUF-1] = '\0';
  if (num == 0) {
    buf[i--] = '0';
    return (buf+i+1);
  }
  if (pad < 0)
    pad = i;
  while ((num != 0 || p > 0) && i >= sign && pad > 0) {
    buf[i--] = '0' + abs (num % 10);
    num /= 10;
    pad--;
  }
  if (sign)
    buf[i--] = '-';
  return (buf+i+1);
}
