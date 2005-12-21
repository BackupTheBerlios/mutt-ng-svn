/** @ingroup core_string */
/**
 * @file core/buffer.c
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: String buffer
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

/** alphabet. @bug duplicate in qp.c */
static const char Hex[] = { "0123456789ABCDEF" };

/**
 * Interpret character as hex and return decimal value.
 * @param c Character.
 * @bug duplicate in qp.c
 * @return
 *   - decimal value upon success
 *   - @c -1 upon error
 */
static inline int dec (unsigned char c) {
  if (c >= '0' && c <= '9')
    return (c - '0');
  if (c >= 'a' && c <= 'f')
    return (10 + (c - 'a'));
  if (c >= 'A' && c <= 'F')
    return (10 + (c - 'A'));
  return (-1);
}

/**
 * Helper for buffer_extract_token(): handle backslash-based specials.
 * This includes
 *    - turning \\[rntfeE] into proper \\r, \\n, \\t, etc.
 *    - turning \\c[A-Za-z] into a control character
 *    - turning \\[0-9]{3} into a character assuming the sequence is octal
 *    - turning \\x[0-9a-f]{2} into a character assuming the sequence is hex
 * @param dst Destination buffer.
 * @param work Working pointer set to end of sequence parsed.
 * @return Success.
 */
static int extract_backslash(buffer_t* dst, char** work) {
  unsigned char ch;
  if (!*(*work))                   /* no character after \ -> error */
    return 0;
  switch (ch = *(*work)++) {
  /* we have \c or \C, i.e. a control character but which one? */
  case 'c':
  case 'C':
    if (!*(*work))                 /* no control character -> error */
      return 0;
    buffer_add_ch(dst,(toupper((unsigned char) *(*work)++)-'@')&0x7f);
    break;
  /* turn the \[rntfe] two bytes fake into a one byte real one */
  case 'r': buffer_add_ch(dst,'\r'); break;
  case 'n': buffer_add_ch(dst,'\n'); break;
  case 't': buffer_add_ch(dst,'\t'); break;
  case 'f': buffer_add_ch(dst,'\f'); break;
  case 'E':
  case 'e': buffer_add_ch(dst,'\033'); break;
  default:
    /* by default, for \[0-9]{3} we interpret as octal */
    if (isdigit ((unsigned char) ch) &&
        isdigit ((unsigned char) *(*work)) && *(*work) >= '0' && *(*work) <= '7' &&
        isdigit ((unsigned char) *((*work)+1)) && *((*work)+1) >= '0' && *((*work)+1) <= '7') {
      buffer_add_ch(dst,(ch<<6)+(*(*work)<<3)+*((*work)+1)-3504);
      (*work)+=2;
    } else if (ch == 'x' &&
               dec((unsigned char) *(*work)) >= 0 &&
               dec((unsigned char) *((*work)+1)) >= 0) {
      buffer_add_ch(dst,(dec ((unsigned char) *(*work)) << 4) |
                        (dec ((unsigned char) *((*work)+1))));
      (*work)+=2;
    }
    else
      buffer_add_ch(dst,ch);
  }
  return 1;
}

/**
 * Helper for buffer_extract_token(): handle ^-based specials.
 * This includes
 *    - turning ^^ into ^
 *    - turning ^[ into escape
 *    - turning ^[A-Za-z] into control character
 *    - just copying ^. otherwise
 * @param dst Destination buffer.
 * @param work Working pointer set to end of sequence parsed.
 * @return Success.
 */
static int extract_control(buffer_t* dst, char** work) {
  unsigned char ch;
  if (!*(*work))
    return 0;                               /* no character after ^ -> error */
  ch = *(*work)++;
  if (ch == '^')                            /* '^^' maps to '^' */
    buffer_add_ch(dst,ch);
  else if (ch == '[')                       /* '^\[' maps to esc */
    buffer_add_ch(dst,'\033');
  else if (isalpha ((unsigned char) ch))    /* '^[A-Za-z]' maps to CTRL */
    buffer_add_ch(dst,toupper((unsigned char)ch)-'@');
  else {                                    /* default: just copy */
    buffer_add_ch(dst,'^');
    buffer_add_ch(dst,ch);
  }
  return 1;
}

/**
 * Helper for buffer_extract_token(): handle variables.
 * It supports the following types of notations:
 *      - $NAME and
 *      - ${NAME}
 * For each name found, it tries to replace the sequence with the value
 * of the given environment variable. If it could not be found and a
 * callback is given, it tries to expand it via the callback.
 * @param dst Destination buffer.
 * @param work Working pointer set to end of sequence parsed.
 * @param expand Optional callback for expansion.
 * @return 1.
 */
static int extract_var(buffer_t* dst, char** work, int(*expand)(buffer_t*,buffer_t*)) {
  buffer_t var;
  char* pc, *val;
  buffer_init(&var);

  if (*(*work) == '{') {
    /* name is enclosed in {} */
    (*work)++;
    if ((pc = strchr ((*work), '}'))) {
      _buffer_add_str(&var,(*work),pc-(*work));
      (*work) = pc+1;
    }
  } else {
    /* name is not enclosed in {}: forward while valid chars found */
    for (pc = (*work); isalnum ((unsigned char) *pc) || *pc == '_'; pc++);
    _buffer_add_str(&var,(*work),pc-(*work));
    (*work) = pc;
  }

  if (!var.len)
    return 1;
  if ((val = getenv(var.str)))
    _buffer_add_str(dst,val,-1);
  else if (expand)
    expand(dst,&var);
  buffer_free(&var);
  return 1;
}

static int extract_backticks(buffer_t* dst, char** work) {
  char* p = *work;
  buffer_t cmd, tmp;
  pid_t pid;
  FILE* fp;
  unsigned int rc;

  do {
    if ((p = strpbrk (p, "\\`"))) {
      /* skip any quoted chars */
      if (*p == '\\')
        p += 2;
    }
  } while (p && *p != '`');

  /* mistmatched backticks? */
  if (!p) return 0;

  buffer_init(&cmd);

  buffer_add_str(&cmd,*work,p-*work);

  if ((pid = command_filter(cmd.str,NULL,&fp,NULL))<0) {
    buffer_free(&cmd);
    return 0;
  }

  buffer_free(&cmd);

  *work = p+1;

  /* read line: io_readline() shrinks buffer -> tmp copy */
  buffer_init(&tmp);
  rc = io_readline(&tmp,fp);
  fclose(fp);
  command_filter_wait(pid);

  buffer_add_buffer(dst,&tmp);
  buffer_free(&tmp);

  return 1;
}

size_t buffer_extract_token  (buffer_t* dst, buffer_t* token, int flags,
                              int(*expand)(buffer_t*,buffer_t*)) {
  if (!dst || !token || !token->len)
    return 0;
  return buffer_extract_token2(dst,(const char*)token->str,flags,expand);
}

size_t buffer_extract_token2 (buffer_t* dst, const char* token, int flags,
                              int(*expand)(buffer_t*,buffer_t*)) { 
  char ch;
  char qc=0;    /* quote character */
  char* work;   /* working pointer into token->str */

  if (!dst || !token)
    return 0;

  work = (char*)token;

  SKIPWS(work);
  while ((ch = *work)) {
    if (!qc) {
      if ((ISSPACE (ch) && !(flags & M_TOKEN_SPACE)) ||
          (ch == '#' && !(flags & M_TOKEN_COMMENT)) ||
          (ch == '=' && (flags & M_TOKEN_EQUAL)) ||
          (ch == ';' && !(flags & M_TOKEN_SEMICOLON)) ||
          ((flags & M_TOKEN_PATTERN) && strchr ("~=!|", ch)))
        break;
    }
    work++;

    if (ch == qc)
      qc = 0;                   /* end of quote */
    else if (!qc && (ch == '\'' || ch == '"') && !(flags & M_TOKEN_QUOTE))
      qc = ch;
    else if (ch == '\\' && qc != '\'') {
      if (!extract_backslash(dst,&work))
        return (const char*)work-token;
    }
    else if (ch == '^' && (flags & M_TOKEN_CONDENSE)) {
      if (!extract_control(dst,&work))
        return (const char*)work-token;
    }
    else if (ch == '$' && (!qc || qc == '"')
             && (*work == '{' || isalpha ((unsigned char) *work))) {
      if (!extract_var(dst,&work,expand))
        return (const char*)work-token;
    }
    else if (ch == '`' && (!qc || qc == '"')) {
      if (!extract_backticks(dst,&work))
        return (const char*)work-token;
    }
    else
      buffer_add_ch(dst,ch);
  }
  SKIPWS(work);
  return (const char*)work-token;
}
