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
        isdigit ((unsigned char) *(*work)) &&
        isdigit ((unsigned char) *((*work)+1))) {
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
      buffer_add_str(&var,(*work),pc-(*work));
      (*work) = pc+1;
    }
  } else {
    /* name is not enclosed in {}: forward while valid chars found */
    for (pc = (*work); isalnum ((unsigned char) *pc) || *pc == '_'; pc++);
    buffer_add_str(&var,(*work),pc-(*work));
    (*work) = pc;
  }

  if (!var.len)
    return 1;
  if ((val = getenv(var.str)))
    buffer_add_str(dst,val,-1);
  else if (expand)
    expand(dst,&var);
  buffer_free(&var);
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
#if 0
    else if (ch == '`' && (!qc || qc == '"')) {
      FILE *fp;
      pid_t pid;
      char *cmd, *ptr;
      size_t expnlen;
      BUFFER expn;
      int line = 0;

      pc = tok->dptr;
      do {
        if ((pc = strpbrk (pc, "\\`"))) {
          /* skip any quoted chars */
          if (*pc == '\\')
            pc += 2;
        }
      } while (pc && *pc != '`');
      if (!pc) {
        debug_print (1, ("mismatched backtics\n"));
        return (-1);
      }
      cmd = str_substrdup (tok->dptr, pc);
      if ((pid = mutt_create_filter (cmd, NULL, &fp, NULL)) < 0) {
        debug_print (1, ("unable to fork command: %s\n", cmd));
        mem_free (&cmd);
        return (-1);
      }
      mem_free (&cmd);

      tok->dptr = pc + 1;

      /* read line */
      memset (&expn, 0, sizeof (expn));
      expn.data = mutt_read_line (NULL, &expn.dsize, fp, &line);
      fclose (fp);
      mutt_wait_filter (pid);

      /* if we got output, make a new string consiting of the shell ouptput
         plus whatever else was left on the original line */
      /* BUT: If this is inside a quoted string, directly add output to 
       * the token */
      if (expn.data && qc) {
        mutt_buffer_addstr (dest, expn.data);
        mem_free (&expn.data);
      }
      else if (expn.data) {
        expnlen = str_len (expn.data);
        tok->dsize = expnlen + str_len (tok->dptr) + 1;
        ptr = mem_malloc (tok->dsize);
        memcpy (ptr, expn.data, expnlen);
        strcpy (ptr + expnlen, tok->dptr);      /* __STRCPY_CHECKED__ */
        if (tok->destroy)
          mem_free (&tok->data);
        tok->data = ptr;
        tok->dptr = ptr;
        tok->destroy = 1;       /* mark that the caller should destroy this data */
        ptr = NULL;
        mem_free (&expn.data);
      }
    }
#endif
    else
      buffer_add_ch(dst,ch);
  }
  SKIPWS(work);
  return (const char*)work-token;
}
