/** @ingroup core_string */
/**
 * @file core/buffer_format.c
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: String buffer
 */
#include <string.h>

#include "buffer.h"

int buffer_format (buffer_t* dst, buffer_t* src,
                   int callback(buffer_t* dst, buffer_t* fmt, unsigned char c)) {
  char* p, *end;
  buffer_t fmt;
  int ret = 0;

  if (!dst || !src) return -1;
  buffer_shrink(dst,0);
  p = src->str;
  while (p-src->str < (signed)src->len) {
    if (*p == '%') {
      p++;
      if (*p && *p == '%') {
        p++;
        buffer_add_ch(dst,'%');
        continue;
      }
      end = p;
      while (*end && !isalpha(*end)) end++;
      if (!end) return -1;
      buffer_init(&fmt);
      if (end!=p) {
        fmt.str = p;
        fmt.len = fmt.size = end-p;
      }
      if (!callback(dst,&fmt,*end))
        return -1;
      else
        ret++;
      p = end+1;
      continue;
    }
    buffer_add_ch(dst,*p);
    p++;
  }
  return ret;
}
