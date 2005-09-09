/*
 * Parts were written/modified by:
 * Andreas Krennmair <ak@synflood.at>
 * Peter J. Holzer <hjp@hjp.net>
 * Rocco Rutte <pdmef@cs.tu-berlin.de>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "mutt.h"
#include "mutt_curses.h"
#include "ascii.h"
#include "handler.h"
#include "state.h"
#include "lib.h"

#include "lib/mem.h"
#include "lib/intl.h"
#include "lib/str.h"
#include "lib/debug.h"

#define FLOWED_MAX 77

static int get_quote_level (char *line)
{
  int quoted;

  for (quoted = 0; line[quoted] == '>'; quoted++);
  return quoted;
}

static void print_flowed_line (char *line, STATE * s, int ql) {
  int width;
  char *pos, *oldpos;
  int len = str_len (line);
  int i;

  if (MaxLineLength > 0) {
    width = MaxLineLength - WrapMargin - ql - 1;
    if (!(s->flags & M_REPLYING) && option (OPTSTUFFQUOTED))
      --width;
    if (width < 0)
      width = MaxLineLength;
  }
  else {
    if (option (OPTMBOXPANE))
      width = COLS - SidebarWidth - WrapMargin - ql - 1;
    else
      width = COLS - WrapMargin - ql - 1;

    if (!(s->flags & M_REPLYING) && option (OPTSTUFFQUOTED))
      --width;
    if (width < 0)
      width = COLS;
  }

  if (str_len (line) == 0) {
    if (!(s->flags & M_REPLYING) || option (OPTQUOTEEMPTY)) {
      if (s->prefix)
        state_puts(s->prefix,s);
      for (i=0;i<ql;++i) state_putc('>',s);
      if (!(s->flags & M_REPLYING) && option(OPTSTUFFQUOTED))
        state_putc(' ',s);
    }
    state_putc('\n',s);
    return;
  }

  pos = line + width;
  oldpos = line;

  for (; oldpos < line + len; pos += width) {
    /* only search a new position when we're not over
     * the end of the string w/ pos */
    if (pos < line + len) {
      if (*pos == ' ') {
        debug_print (4, ("f=f: found space directly at width\n"));
        *pos = '\0';
        ++pos;
      }
      else {
        char *save = pos;
        debug_print (4, ("f=f: need to search for space\n"));

        while (pos >= oldpos && *pos != ' ') {
          --pos;
        }
        if (pos < oldpos) {
          debug_print (4, ("f=f: no space found while searching "
                           "to left; going right\n"));
          pos = save;
          while (pos < line + len && *pos && *pos != ' ') {
            ++pos;
          }
          debug_print (4, ("f=f: found space at pos %d\n", pos-line));
        } else {
          debug_print (4, ("f=f: found space while searching to left\n"));
        }
        *pos = '\0';
        ++pos;
      }
    }
    else {
      debug_print (4, ("f=f: line completely fits on screen\n"));
    }
    if (s->prefix)
      state_puts (s->prefix, s);

    for (i = 0; i < ql; ++i)
      state_putc ('>', s);
    if (!(s->flags & M_REPLYING) && option (OPTSTUFFQUOTED) &&
        (ql > 0 || s->prefix))
      state_putc (' ', s);
    state_puts (oldpos, s);
    /* fprintf(stderr,"print_flowed_line: `%s'\n",oldpos); */
    if (pos < line + len)
      state_putc (' ', s);
    state_putc ('\n', s);
    oldpos = pos;
  }
}

int rfc3676_handler (BODY * a, STATE * s) {
  int bytes = a->length;
  char buf[LONG_STRING];
  char *curline = str_dup ("");
  char *t = NULL;
  unsigned int curline_len = 1,
               quotelevel = 0, newql = 0;
  int buf_off, buf_len;
  int delsp = 0, fixed = 0;

  /* respect DelSP of RfC3676 only with f=f parts */
  if ((t = (char*) mutt_get_parameter ("delsp", a->parameter))) {
    delsp = str_len (t) == 3 && ascii_strncasecmp (t, "yes", 3) == 0;
    t = NULL;
  }

  debug_print (2, ("f=f: DelSp: %s\n", delsp ? "yes" : "no"));

  while (bytes > 0 && fgets (buf, sizeof (buf), s->fpin)) {

    buf_len = str_len (buf);
    bytes -= buf_len;

    newql = get_quote_level (buf);

    /* a change of quoting level in a paragraph - shouldn't happen, 
     * but has to be handled - see RFC 3676, sec. 4.5.
     */
    if (newql != quotelevel && curline && *curline) {
      print_flowed_line (curline, s, quotelevel);
      *curline = '\0';
      curline_len = 1;
    }
    quotelevel = newql;

    /* XXX - If a line is longer than buf (shouldn't happen), it is split.
     * This will almost always cause an unintended line break, and 
     * possibly a change in quoting level. But that's better than not
     * displaying it at all.
     */
    if ((t = strrchr (buf, '\n')) || (t = strrchr (buf, '\r'))) {
      *t = '\0';
      buf_len = t - buf;
    }

    buf_off = newql;
    /* respect space-stuffing */
    if (buf[buf_off] == ' ')
      buf_off++;

    /* for DelSp=yes, we need to strip one SP prior to CRLF
     * which may make the line look like fixed although it wasn't
     * so keep this in mind for later processing */
    fixed = buf_len == 0 || buf[buf_len - 1] != ' ' ||
            (strcmp(buf + buf_off, "-- ") == 0);

    if (delsp && buf_len >= 1 && buf[buf_len-1] == ' ')
      buf[--buf_len] = '\0';

    /* we're here when last space removed 'cause of DelSp was
     * the last space and there isn't more -> done */
    if ((buf_len - buf_off) < 0) {
      print_flowed_line (curline, s, quotelevel);
      *curline = '\0';
      curline_len = 1;
      continue;
    }

    /* signature separator also flushes the previous paragraph */
    if (strcmp(buf + buf_off, "-- ") == 0 && curline && *curline) {
      print_flowed_line (curline, s, quotelevel);
      *curline = '\0';
      curline_len = 1;
    }

    mem_realloc (&curline, curline_len + buf_len - buf_off);
    strcpy (curline + curline_len - 1, buf + buf_off);
    curline_len += buf_len - buf_off;

    /* if this was a fixed line the paragraph is finished */
    if (fixed) {
      print_flowed_line (curline, s, quotelevel);
      *curline = '\0';
      curline_len = 1;
    }

  }
  mem_free (&curline);
  return (0);
}

void rfc3676_space_stuff (HEADER* hdr) {
#if DEBUG
  int lc = 0;
  size_t len = 0;
  unsigned char c = '\0';
#endif
  FILE* in = NULL, *out = NULL;
  char buf[LONG_STRING];
  char tmpfile[_POSIX_PATH_MAX];

  if (!hdr || !hdr->content || !hdr->content->filename)
    return;

  debug_print (2, ("f=f: postprocess %s\n", hdr->content->filename));
  if ((in = safe_fopen (hdr->content->filename, "r")) == NULL)
    return;
  mutt_mktemp (tmpfile);
  if ((out = safe_fopen (tmpfile, "w+")) == NULL) {
    fclose (in);
    return;
  }

  while (fgets (buf, sizeof (buf), in)) {
    if (ascii_strncmp ("From ", buf, 4) == 0 || buf[0] == ' ') {
      fputc (' ', out);
#if DEBUG
      lc++;
      len = str_len (buf);
      if (len > 0) {
        c = buf[len-1];
        buf[len-1] = '\0';
      }
      debug_print (4, ("f=f: line %d needs space-stuffing: '%s'\n",
                       lc, buf));
      if (len > 0)
        buf[len-1] = c;
#endif
    }
    fputs (buf, out);
  }
  fclose (in);
  fclose (out);
  mutt_set_mtime (hdr->content->filename, tmpfile);
  unlink (hdr->content->filename);
  str_replace (&hdr->content->filename, tmpfile);
}
