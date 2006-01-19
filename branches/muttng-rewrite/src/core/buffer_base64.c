/** @ingroup core_conv */
/**
 * @file core/buffer_base64.c
 * @author Felix von Leitner <felix-libowfat@fefe.de>
 * @brief Implementation: Base64 conversions
 *
 * This code is based on the functions scan_base64() and fmt_base64() 
 * of libowfat (see http://www.fefe.de/libowfat/).
 *
 * libowfat was written by Felix von Leitner <felix-libowfat@fefe.de> 
 * which is licensed under the GNU General Public License.
 *
 * This file is published under the GNU General Public License.
 */
#include "buffer.h"

/** alphabet */
static const char base64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

/**
 * convert character to decimal.
 * @param x Character.
 * @return
 *   - if @c x matches <tt>[A-Za-z0-9+/]</tt>: numerical value
 *   - if @c x is \\0 or =: -2
 *   - -1 otherwise
 */
static inline int dec(unsigned char x) {
  if (x>='A' && x<='Z') return x-'A';
  if (x>='a' && x<='z') return x-'a'+26;
  if (x>='0' && x<='9') return x-'0'+26+26;
  switch (x) {
  case '+': return 62;
  case '/': return 63;
  case '\0':
  case '=': return -2;
  default: return -1;
  }
}

int buffer_base64_decode(buffer_t * dest,const buffer_t * src, size_t* chars) {
  unsigned short tmp=0,bits=0,rc=1;
  register const unsigned char* s=(const unsigned char*) src->str;
  if (chars)
    *chars = 0;
  for (;;) {
    int a=dec(*s);
    switch(a) {
    case -1:
      rc = 0;
      if (chars)
        *chars = (const char*)s-src->str;
      /* fallthrough */
    case -2:
      while (*s=='=') ++s;
      return rc;
      break;
    default:
      break;
    }
    tmp=(tmp<<6)|a; bits+=6;
    ++s;
    if (bits>=8) {
      buffer_add_ch(dest,(tmp>>(bits-=8)));
    }
  }
  return rc;
}

void buffer_base64_encode(buffer_t * dest,const buffer_t * src) {
  register const unsigned char* s=(const unsigned char*) src->str;
  unsigned short bits=0,temp=0;
  unsigned long written=0,i;
  unsigned int len = src->len;
  if (!dest) return;
  for (i=0; i<len; ++i) {
    temp<<=8; temp+=s[i]; bits+=8;
    while (bits>6) {
      buffer_add_ch(dest,base64[((temp>>(bits-6))&63)]);
      ++written; bits-=6;
    }
  }
  if (bits) {
    temp<<=(6-bits);
    buffer_add_ch(dest,base64[temp&63]);
    ++written;
  }
  while (written&3) { buffer_add_ch(dest,'='); ++written; }
}
