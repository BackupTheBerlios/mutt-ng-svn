/** @ingroup core_string */
/**
 * @file core/buffer_token.c
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

/**
 * Helper for buffer_extract_token(): handle backticks. This means to
 * run a command and replace the backtick sequence with its output.
 * @param dst Destination buffer.
 * @param work Working pointer set to end of sequence parsed.
 * @return Success.
 */
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

size_t buffer_extract_token (buffer_t* dst, buffer_t* token, int flags) {
  if (!dst || !token || !token->len)
    return 0;
  return buffer_extract_token2(dst,(const char*)token->str,flags);
}

size_t buffer_extract_token2 (buffer_t* dst, const char* token, int flags) {
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
    else
      buffer_add_ch(dst,ch);
  }
  SKIPWS(work);
  return (const char*)work-token;
}

size_t buffer_expand (buffer_t* dst, buffer_t* src,
                      int(*expand)(buffer_t* dst,buffer_t* var)) {
  char* p;
  unsigned char quote = '\0';

  if (!dst || !src) return -1;
  buffer_grow(dst,src->len);
  buffer_shrink(dst,0);
  p = src->str;
  while (p-src->str < (signed)src->len) {
    if (*p == '\'' || *p == '"')
      quote = (*p==quote)?'\0':*p;
    else {
      if (*p == '$' && quote != '\'') {
        p++;
        extract_var(dst,&p,expand);
        continue;
      }
      if (*p == '`' && quote != '\'') {
        p++;
        extract_backticks(dst,&p);
        continue;
      }
      if (*p == '\\')
        buffer_add_ch(dst,*p++);
    }
    buffer_add_ch(dst,*p++);
  }
  /* if quote left after end, we have mismached quotes */
  if (quote!='\0') return -1;
  return p-src->str;
}
