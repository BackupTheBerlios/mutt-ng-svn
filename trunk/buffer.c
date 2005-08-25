/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */
#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "buffer.h"

#include "lib/mem.h"
#include "lib/str.h"
#include "lib/debug.h"

/*
 * Creates and initializes a BUFFER*. If passed an existing BUFFER*,
 * just initializes. Frees anything already in the buffer.
 *
 * Disregards the 'destroy' flag, which seems reserved for caller.
 * This is bad, but there's no apparent protocol for it.
 */
BUFFER *mutt_buffer_init (BUFFER * b)
{
  if (!b) {
    b = mem_malloc (sizeof (BUFFER));
    if (!b)
      return NULL;
  }
  else {
    mem_free(&b->data);
  }
  memset (b, 0, sizeof (BUFFER));
  return b;
}

/*
 * Creates and initializes a BUFFER*. If passed an existing BUFFER*,
 * just initializes. Frees anything already in the buffer. Copies in
 * the seed string.
 *
 * Disregards the 'destroy' flag, which seems reserved for caller.
 * This is bad, but there's no apparent protocol for it.
 */
BUFFER *mutt_buffer_from (BUFFER * b, char *seed)
{
  if (!seed)
    return NULL;

  b = mutt_buffer_init (b);
  b->data = str_dup (seed);
  b->dsize = str_len (seed);
  b->dptr = (char *) b->data + b->dsize;
  return b;
}

void mutt_buffer_addstr (BUFFER * buf, const char *s)
{
  mutt_buffer_add (buf, s, str_len (s));
}

void mutt_buffer_addch (BUFFER * buf, char c)
{
  mutt_buffer_add (buf, &c, 1);
}

void mutt_buffer_free (BUFFER ** p)
{
  if (!p || !*p)
    return;

  mem_free (&(*p)->data);
  /* dptr is just an offset to data and shouldn't be freed */
  mem_free (p);
}

/* dynamically grows a BUFFER to accomodate s, in increments of 128 bytes.
 * Always one byte bigger than necessary for the null terminator, and
 * the buffer is always null-terminated */
void mutt_buffer_add (BUFFER * buf, const char *s, size_t len)
{
  size_t offset;

  if (buf->dptr + len + 1 > buf->data + buf->dsize) {
    offset = buf->dptr - buf->data;
    buf->dsize += len < 128 ? 128 : len + 1;
    mem_realloc ((void **) &buf->data, buf->dsize);
    buf->dptr = buf->data + offset;
  }
  memcpy (buf->dptr, s, len);
  buf->dptr += len;
  *(buf->dptr) = '\0';
}

int mutt_extract_token (BUFFER * dest, BUFFER * tok, int flags)
{
  char ch;
  char qc = 0;                  /* quote char */
  char *pc;

  /* reset the destination pointer to the beginning of the buffer */
  dest->dptr = dest->data;

  SKIPWS (tok->dptr);
  while ((ch = *tok->dptr)) {
    if (!qc) {
      if ((ISSPACE (ch) && !(flags & M_TOKEN_SPACE)) ||
          (ch == '#' && !(flags & M_TOKEN_COMMENT)) ||
          (ch == '=' && (flags & M_TOKEN_EQUAL)) ||
          (ch == ';' && !(flags & M_TOKEN_SEMICOLON)) ||
          ((flags & M_TOKEN_PATTERN) && strchr ("~!|", ch)))
        break;
    }

    tok->dptr++;

    if (ch == qc)
      qc = 0;                   /* end of quote */
    else if (!qc && (ch == '\'' || ch == '"') && !(flags & M_TOKEN_QUOTE))
      qc = ch;
    else if (ch == '\\' && qc != '\'') {
      if (!*tok->dptr)
        return -1;              /* premature end of token */
      switch (ch = *tok->dptr++) {
      case 'c':
      case 'C':
        if (!*tok->dptr)
          return -1;            /* premature end of token */
        mutt_buffer_addch (dest, (toupper ((unsigned char) *tok->dptr)
                                  - '@') & 0x7f);
        tok->dptr++;
        break;
      case 'r':
        mutt_buffer_addch (dest, '\r');
        break;
      case 'n':
        mutt_buffer_addch (dest, '\n');
        break;
      case 't':
        mutt_buffer_addch (dest, '\t');
        break;
      case 'f':
        mutt_buffer_addch (dest, '\f');
        break;
      case 'e':
        mutt_buffer_addch (dest, '\033');
        break;
      default:
        if (isdigit ((unsigned char) ch) &&
            isdigit ((unsigned char) *tok->dptr) &&
            isdigit ((unsigned char) *(tok->dptr + 1))) {

          mutt_buffer_addch (dest,
                             (ch << 6) + (*tok->dptr << 3) + *(tok->dptr +
                                                               1) - 3504);
          tok->dptr += 2;
        }
        else
          mutt_buffer_addch (dest, ch);
      }
    }
    else if (ch == '^' && (flags & M_TOKEN_CONDENSE)) {
      if (!*tok->dptr)
        return -1;              /* premature end of token */
      ch = *tok->dptr++;
      if (ch == '^')
        mutt_buffer_addch (dest, ch);
      else if (ch == '[')
        mutt_buffer_addch (dest, '\033');
      else if (isalpha ((unsigned char) ch))
        mutt_buffer_addch (dest, toupper ((unsigned char) ch) - '@');
      else {
        mutt_buffer_addch (dest, '^');
        mutt_buffer_addch (dest, ch);
      }
    }
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
    else if (ch == '$' && (!qc || qc == '"')
             && (*tok->dptr == '{' || isalpha ((unsigned char) *tok->dptr))) {
      char *env = NULL, *var = NULL;

      if (*tok->dptr == '{') {
        tok->dptr++;
        if ((pc = strchr (tok->dptr, '}'))) {
          var = str_substrdup (tok->dptr, pc);
          tok->dptr = pc + 1;
        }
      }
      else {
        for (pc = tok->dptr; isalnum ((unsigned char) *pc) || *pc == '_';
             pc++);
        var = str_substrdup (tok->dptr, pc);
        tok->dptr = pc;
      }
      if (var) {
        char tmp[STRING];
        if ((env = getenv (var)) || 
            (mutt_option_value (var, tmp, sizeof (tmp)) && (env = tmp)))
          mutt_buffer_addstr (dest, env);
      }
      mem_free (&var);
    }
    else
      mutt_buffer_addch (dest, ch);
  }
  mutt_buffer_addch (dest, 0);  /* terminate the string */
  SKIPWS (tok->dptr);
  return 0;
}

