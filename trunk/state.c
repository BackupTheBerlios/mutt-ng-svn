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
#include <stdio.h>

#include "mutt.h"
#include "state.h"

static void state_prefix_put (const char *d, size_t dlen, STATE * s)
{
  if (s->prefix)
    while (dlen--)
      state_prefix_putc (*d++, s);
  else
    fwrite (d, dlen, 1, s->fpout);
}

void mutt_convert_to_state (iconv_t cd, char *bufi, size_t * l, STATE * s)
{
  char bufo[BUFO_SIZE];
  ICONV_CONST char *ib;
  char *ob;
  size_t ibl, obl;

  if (!bufi) {
    if (cd != (iconv_t) (-1)) {
      ob = bufo, obl = sizeof (bufo);
      iconv (cd, 0, 0, &ob, &obl);
      if (ob != bufo)
        state_prefix_put (bufo, ob - bufo, s);
    }
    if (Quotebuf[0] != '\0')
      state_prefix_putc ('\n', s);
    return;
  }

  if (cd == (iconv_t) (-1)) {
    state_prefix_put (bufi, *l, s);
    *l = 0;
    return;
  }

  ib = bufi, ibl = *l;
  for (;;) {
    ob = bufo, obl = sizeof (bufo);
    mutt_iconv (cd, &ib, &ibl, &ob, &obl, 0, "?");
    if (ob == bufo)
      break;
    state_prefix_put (bufo, ob - bufo, s);
  }
  memmove (bufi, ib, ibl);
  *l = ibl;
}

void state_prefix_putc (char c, STATE * s)
{
  if (s->flags & M_PENDINGPREFIX) {
    int i;

    i = str_len (Quotebuf);
    Quotebuf[i++] = c;
    Quotebuf[i] = '\0';
    if (i == sizeof (Quotebuf) - 1 || c == '\n') {
      char buf[2 * SHORT_STRING];
      int j = 0, offset = 0;
      regmatch_t pmatch[1];

      state_reset_prefix (s);
      while (regexec
             ((regex_t *) QuoteRegexp.rx, &Quotebuf[offset], 1, pmatch,
              0) == 0)
        offset += pmatch->rm_eo;

      if (!option (OPTQUOTEEMPTY) && Quotebuf[offset] == '\n') {
        buf[0] = '\n';
        buf[1] = '\0';
      }
      else if (option (OPTQUOTEQUOTED) && offset) {
        for (i = 0; i < offset; i++)
          if (Quotebuf[i] != ' ')
            j = i;
        strncpy (buf, Quotebuf, j + 1);
        strcpy (buf + j + 1, Quotebuf + j);
      }
      else
        snprintf (buf, sizeof (buf), "%s%s", NONULL (s->prefix), Quotebuf);

      state_puts (buf, s);
    }
  }
  else
    state_putc (c, s);

  if (c == '\n') {
    state_set_prefix (s);
    Quotebuf[0] = '\0';
  }
}

int state_printf (STATE * s, const char *fmt, ...)
{
  int rv;
  va_list ap;

  va_start (ap, fmt);
  rv = vfprintf (s->fpout, fmt, ap);
  va_end (ap);

  return rv;
}

void state_mark_attach (STATE * s)
{
  if ((s->flags & M_DISPLAY) && !str_cmp (Pager, "builtin"))
    state_puts (AttachmentMarker, s);
}

void state_attach_puts (const char *t, STATE * s)
{
  if (*t != '\n')
    state_mark_attach (s);
  while (*t) {
    state_putc (*t, s);
    if (*t++ == '\n' && *t)
      if (*t != '\n')
        state_mark_attach (s);
  }
}
