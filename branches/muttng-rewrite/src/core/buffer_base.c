/** @ingroup core_string */
/**
 * @file core/buffer_base.c
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: String buffer
 *
 * This file is published under the GNU Lesser General Public License.
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "buffer.h"
#include "mem.h"
#include "str.h"
#include "conv.h"
#include "command.h"
#include "io.h"

/** we grow buffer on demand by this size */
#define BUF_INC         256

/** alphabet of numbers */
static const char* Alph = "0123456789abcdef";

/**
 * Buffer size for converting int to string via conv_itoa().
 * As we may convert to binary:
 *   - 128 for bits
 *   - we may have a sign: +1
 *   - trailing \\0: +1
 */
#define NUMBUF  130

/**
 * Convert signed int to string.
 * @b NOTE: when padding is requested, the number will have exactly the
 * number of digits given, i.e. padding does @b not mean to optionally
 * fill with leading zeros to at least get a given lenght but to always
 * exactly get the given length.
 * @param buf Buffer of size NUMBUF.
 * @param num Number to convert.
 * @param pad To how many digits to 0-pad the number. If negative,
 *            the number will be converted as-is.
 * @param base Number base (2, 8, 10, 16).
 * @return Offset into buf.
 * @test buffer_tests::test_buffer_add_snum2().
 */
static inline char* conv_itoa2 (char* buf, signed long num, short pad, short base) {
  register unsigned short i = NUMBUF-2;
  unsigned short sign = num<0;

  memset(buf,'0',NUMBUF-1);
  buf[NUMBUF-1] = '\0';

  if (num == 0 && pad < 0)
    return buf+NUMBUF-2;

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
  while (num != 0 && i >= sign) {
    buf[i--] = Alph[abs (num % base)];
    num /= base;
  }
  if (pad<0) {
    if (sign)
      buf[i--] = '-';
    return buf+i+1;
  }
  i = NUMBUF-pad-1<sign ? 0 : (NUMBUF-pad-1)-sign;
  if (sign)
    buf[i] = '-';
  return buf+i;
}

/**
 * Convert signed int to string.
 * @b NOTE: when padding is requested, the number will have exactly the
 * number of digits given, i.e. padding does @b not mean to optionally
 * fill with leading zeros to at least get a given lenght but to always
 * exactly get the given length.
 * @param buf Buffer of size NUMBUF.
 * @param num Number to convert.
 * @param pad To how many digits to 0-pad the number. If negative,
 *            the number will be converted as-is.
 * @param base Number base (2, 8, 10, 16).
 * @return Offset into buf.
 * @test buffer_tests::test_buffer_add_snum2().
 */
static inline char* conv_uitoa2 (char* buf, unsigned long num, short pad, short base) {
  register unsigned short i = NUMBUF-2;

  memset(buf,'0',NUMBUF-1);
  buf[NUMBUF-1] = '\0';

  if (num == 0 && pad < 0)
    return buf+NUMBUF-2;

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
  while (num != 0 && i != 0) {
    buf[i--] = Alph[abs (num % base)];
    num /= base;
  }
  if (pad<0)
    return buf+i+1;
  return (NUMBUF-pad-1<0)?buf:buf+(NUMBUF-pad-1);
}

/** @copydoc buffer_add_str(). */
static inline void _buffer_add_str(buffer_t* buffer, const char* s, int len) {
  if (!buffer)
    return;
  if (len < 0)
    len = str_len (s);
  if (!len)
    return;
  if (len+buffer->len+1 >= buffer->size) {
    buffer->size += (len > BUF_INC ? len : BUF_INC) + (buffer->size == 0);
    mem_realloc (&buffer->str, buffer->size);
  }
  memcpy (buffer->str + buffer->len, s, len);
  buffer->len += len;
  buffer->str[buffer->len] = '\0';
}

void buffer_add_str (buffer_t* buffer, const char* s, int len) {
  _buffer_add_str(buffer,s,len);
}

void buffer_add_ch (buffer_t* buffer, unsigned char ch) {
  _buffer_add_str(buffer,(const char*)&ch,1);
}

void buffer_add_snum2 (buffer_t* buffer, long num, short pad, short base) {
  char buf[NUMBUF];
  _buffer_add_str (buffer, conv_itoa2 (buf, num, pad, base), -1);
}

void buffer_add_unum2 (buffer_t* buffer, unsigned long num, short pad, short base) {
  char buf[NUMBUF];
  _buffer_add_str (buffer, conv_uitoa2 (buf, num, pad, base), -1);
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
    _buffer_add_str (dst, (const char*) src->str, src->len);
}

void buffer_shrink (buffer_t* buffer, size_t len) {
  if (!buffer) return;
  if (buffer->size && buffer->len+1 > len) {
    buffer->len = len;
    buffer->str[len] = '\0';
  }
}

void buffer_grow (buffer_t* buffer, size_t size) {
  if (!buffer) return;
  if (buffer->size==0) {
    buffer->size = size+1;
    buffer->str = mem_malloc(buffer->size);
    buffer->str[0] = '\0';
    return;
  }
  if (buffer->size >= size+1) return;
  buffer->size = size+1;
  mem_realloc(&buffer->str,buffer->size);
}

unsigned int buffer_chomp(buffer_t * buffer) {
  unsigned int chomp_count = 0;
  if (buffer) {
    while (buffer->len > 0 && (buffer->str[buffer->len-1]=='\r' || buffer->str[buffer->len-1]=='\n')) {
      buffer->str[buffer->len-1]='\0';
      buffer->len--;
      chomp_count++;
    }
  }
  return chomp_count;
}
