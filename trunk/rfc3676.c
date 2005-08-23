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

typedef int handler_f (BODY *, STATE *);
typedef handler_f *handler_t;

#define FLOWED_MAX 77

static int get_quote_level (char *line)
{
  int quoted;

  for (quoted = 0; line[quoted] == '>'; quoted++);
  return quoted;
}

static void print_flowed_line (char *line, STATE * s,
                               int ql, int delsp,
                               int* spaces, int space_len) {
  int width;
  char *pos, *oldpos;
  int len = str_len (line);
  int i;

  if (MaxLineLength > 0) {
    width = MaxLineLength - WrapMargin - ql - 1;
    if (option (OPTSTUFFQUOTED))
      --width;
    if (width < 0)
      width = MaxLineLength;
  }
  else {
    if (option (OPTMBOXPANE))
      width = COLS - SidebarWidth - WrapMargin - ql - 1;
    else
      width = COLS - WrapMargin - ql - 1;

    if (option (OPTSTUFFQUOTED))
      --width;
    if (width < 0)
      width = COLS;
  }

  if (str_len (line) == 0) {
    if (option (OPTQUOTEEMPTY)) {
      if (s->prefix)
        state_puts(s->prefix,s);
      for (i=0;i<ql;++i) state_putc('>',s);
      if (option(OPTSTUFFQUOTED))
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
      /* fprintf(stderr,"if 1\n"); */
      if (*pos == ' ') {
        /* fprintf(stderr,"if 2: good luck! found a space\n"); */
        *pos = '\0';
        ++pos;
      }
      else {
        /* fprintf(stderr,"if 2: else\n"); */
        char *save = pos;

        while (pos >= oldpos && *pos != ' ') {
          /* fprintf(stderr,"pos(%p) > oldpos(%p)\n",pos,oldpos); */
          --pos;
        }
        if (pos < oldpos) {
          /* fprintf(stderr,"wow, no space found,
           * searching the other direction\n"); */
          pos = save;
          while (pos < line + len && *pos && *pos != ' ') {
            /* fprintf(stderr,"pos(%p) < line+len(%p)\n",pos,line+len); */
            ++pos;
          }
          /* fprintf(stderr,"found a space pos = %p\n",pos); */
        }
        *pos = '\0';
        ++pos;
      }
    }
    else {
      /* fprintf(stderr,"if 1 else\n"); */
    }
    if (s->prefix)
      state_puts (s->prefix, s);

    for (i = 0; i < ql; ++i)
      state_putc ('>', s);
    if (option (OPTSTUFFQUOTED) && (ql > 0 || s->prefix))
      state_putc (' ', s);

    if (delsp && spaces && space_len > 0) {
      /* here, we need to character-wise step through the line
       * to eliminate all spaces which were trailing due to DelSp */
      for (i = 0; i < str_len (oldpos); i++) {
        if (oldpos[i] == ' ' && spaces[&(oldpos[i])-line] != 0) {
          debug_print (4, ("DelSp: spaces[%d] forces space removal\n",
                           &(oldpos[i])-line));
          continue;
        }
        /* print space at oldpos[i] if it was non-trailing */
        state_putc (oldpos[i], s);
      }
    } else
      /* for no DelSp, just do whole line as per usual */
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
  unsigned int curline_len = 1, space_len = 1,
               quotelevel = 0, newql = 0;
  int buf_off, buf_len;
  int delsp = 0;
  int* spaces = NULL;

  /* respect DelSP of RfC3676 only with f=f parts */
  if ((t = (char*) mutt_get_parameter ("delsp", a->parameter))) {
    delsp = str_len (t) == 3 && ascii_strncasecmp (t, "yes", 3) == 0;
    t = NULL;
  }

  debug_print (2, ("DelSp: %s\n", delsp ? "yes" : "no"));

  while (bytes > 0 && fgets (buf, sizeof (buf), s->fpin)) {
    buf_len = str_len (buf);
    bytes -= buf_len;

    newql = get_quote_level (buf);

    /* a change of quoting level in a paragraph - shouldn't happen, 
     * but has to be handled - see RFC 3676, sec. 4.5.
     */
    if (newql != quotelevel && curline && *curline) {
      print_flowed_line (curline, s, quotelevel, delsp, spaces, space_len);
      *curline = '\0';
      curline_len = 1;
      space_len = 0;
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

    /* signature separator also flushes the previous paragraph */
    if (strcmp(buf + buf_off, "-- ") == 0 && curline && *curline) {
      print_flowed_line (curline, s, quotelevel, delsp, spaces, space_len);
      *curline = '\0';
      curline_len = 1;
      space_len = 0;
    }

    mem_realloc (&curline, curline_len + buf_len - buf_off);
    mem_realloc (&spaces, (curline_len + buf_len - buf_off)*sizeof (int));
    strcpy (curline + curline_len - 1, buf + buf_off);
    memset (&spaces[space_len], 0, (buf_len - buf_off)*sizeof (int));
    curline_len += buf_len - buf_off;
    space_len += buf_len - buf_off;

    /* if this was a fixed line the paragraph is finished */
    if (buf_len == 0 || buf[buf_len - 1] != ' ' || strcmp(buf + buf_off, "-- ") == 0) {
      print_flowed_line (curline, s, quotelevel, delsp, spaces, space_len);
      *curline = '\0';
      curline_len = 1;
      space_len = 0;
    } else {
      /* if last line we appended had a space and we have DelSp=yes,
       * get a 1 into spaces array at proper position so that
       * print_flowed_line() can handle it; don't kill the space
       * right here 'cause we maybe need soft linebreaks to search for break
       */
      if (delsp && curline && *curline && curline_len-2 >= 0 &&
          curline[curline_len-2] == ' ') {
        debug_print (4, ("DelSp: marking spaces[%d] for later removal\n",
                         curline_len-2));
        spaces[curline_len-2] = 1;
      }
    }

  }
  mem_free (&spaces);
  mem_free (&curline);
  return (0);
}
