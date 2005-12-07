#include "buffer.h"
#include "base64.h"

/*
 * This code is based on the functions scan_base64() and fmt_base64() 
 * of libowfat.
 * libowfat was written by Felix von Leitner <felix-libowfat@fefe.de> 
 * and is licensed under the GNU General Public License.
 */

static const char base64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

static inline int dec(unsigned char x) {
  if (x>='A' && x<='Z') return x-'A';
  if (x>='a' && x<='z') return x-'a'+26;
  if (x>='0' && x<='9') return x-'0'+26+26;
  switch (x) {
  case '+': return 62;
  case '/': return 63;
  default: return -1;
  }
}

unsigned long buffer_decode_base64(buffer_t * dest,const buffer_t * src) {
  unsigned short tmp=0,bits=0;
  register const unsigned char* s=(const unsigned char*) src->str;
  for (;;) {
    int a=dec(*s);
    if (a<0) {
      while (*s=='=') ++s;
      break;
    }
    tmp=(tmp<<6)|a; bits+=6;
    ++s;
    if (bits>=8) {
      buffer_add_ch(dest,(tmp>>(bits-=8)));
    }
  }
  return (const char*)s-src->str;
}

unsigned long buffer_encode_base64(buffer_t * dest,const buffer_t * src) {
  register const unsigned char* s=(const unsigned char*) src->str;
  unsigned short bits=0,temp=0;
  unsigned long written=0,i;
  unsigned int len = src->len;
  if (!dest) return ((len+2)/3)*4;
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
  return written;
}
