/**
 * @ingroup core
 */
/**
 * @file core/buffer.c
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: String buffer
 */
#include <stdlib.h>
#include <string.h>

#include "buffer.h"
#include "mem.h"
#include "str.h"
#include "conv.h"

/** we grow buffer on demond by this size */
#define BUF_INC         256

void buffer_add_str (buffer_t* buffer, const char* s, int len) {
  if (!buffer)
    return;
  if (len < 0)
    len = str_len (s);
  if (!len)
    return;
  if ((int) (buffer->size - buffer->len) < len+1) {
    buffer->size += (len > BUF_INC ? len : BUF_INC) + (buffer->size == 0);
    mem_realloc (&buffer->str, buffer->size);
  }
  memcpy (buffer->str + buffer->len, s, len);
  buffer->len += len;
  buffer->str[buffer->len] = '\0';
}

void buffer_add_ch (buffer_t* buffer, unsigned char ch) {
  char buf[2] = "\0\0";
  buf[0] = ch;
  buffer_add_str (buffer, buf, 1);
}

void buffer_add_num2 (buffer_t* buffer, int num, int pad, int base) {
  char buf[NUMBUF];
  if (!buffer)
    return;
  buffer_add_str (buffer, conv_itoa2 (buf, num, pad, base), -1);
}

int buffer_equal1 (buffer_t* buffer, const char* s, int len) {
  size_t l = (len >= 0 ? (size_t) len : str_len (s));
  if (!buffer || !s)
    return (!buffer && !s);
  return ((len >= 0 || buffer->len == l) && strncmp (buffer->str, s, l) == 0);
}

int buffer_equal2 (buffer_t* buffer1, buffer_t* buffer2) {
  if (!buffer1 || !buffer2)
    return (!buffer1 && !buffer2);
  return (buffer_equal1 (buffer1, buffer2->str, buffer2->len));
}

void buffer_free (buffer_t* buffer) {
  if (!buffer)
    return;
  mem_free (&buffer->str);
  buffer->len = 0;
  buffer->size = 0;
}

void buffer_add_buffer (buffer_t* dst, buffer_t* src) {
  if (src && src->len)
    buffer_add_str (dst, (const char*) src->str, src->len);
}

void buffer_shrink (buffer_t* buffer, size_t len) {
  if (buffer->size && buffer->size > len) {
    buffer->len = len;
    buffer->str[len] = '\0';
  }
}
