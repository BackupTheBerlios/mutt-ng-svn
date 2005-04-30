/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2002 Michael R. Elkins <me@mutt.org>
 *
 * Parts were written/modified by:
 * Nico Golde <nico@ngolde.de>
 * Andreas Krennmair <ak@synflood.at>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "mutt.h"
#include "mutt_curses.h"
#include "keymap.h"
#include "mutt_menu.h"
#include "mapping.h"
#include "sort.h"
#include "pager.h"
#include "attach.h"
#include "mbyte.h"
#include "sidebar.h"
#include "buffy.h"

#include "mx.h"

#ifdef USE_IMAP
#include "imap_private.h"
#endif

#include "mutt_crypt.h"

#include "lib/mem.h"
#include "lib/intl.h"
#include "lib/str.h"
#include "lib/rx.h"
#include "lib/debug.h"

#include <sys/stat.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define ISHEADER(x) ((x) == MT_COLOR_HEADER || (x) == MT_COLOR_HDEFAULT)

#define IsAttach(x) (x && (x)->bdy)
#define IsRecvAttach(x) (x && (x)->bdy && (x)->fp)
#define IsSendAttach(x) (x && (x)->bdy && !(x)->fp)
#define IsMsgAttach(x) (x && (x)->fp && (x)->bdy && (x)->bdy->hdr)
#define IsHeader(x) (x && (x)->hdr && !(x)->bdy)

static const char *Not_available_in_this_menu =
N_("Not available in this menu.");
static const char *Mailbox_is_read_only = N_("Mailbox is read-only.");
static const char *Function_not_permitted_in_attach_message_mode =
N_("Function not permitted in attach-message mode.");

#define CHECK_MODE(x)	if (!(x)) \
			{ \
			  	mutt_flushinp (); \
				mutt_error _(Not_available_in_this_menu); \
				break; \
			}

#define CHECK_READONLY	if (Context->readonly) \
			{ \
				mutt_flushinp (); \
				mutt_error _(Mailbox_is_read_only);	\
				break; \
			}

#define CHECK_ATTACH if(option(OPTATTACHMSG)) \
		     {\
			mutt_flushinp (); \
			mutt_error _(Function_not_permitted_in_attach_message_mode); \
			break; \
		     }

struct q_class_t {
  int length;
  int index;
  int color;
  char *prefix;
  struct q_class_t *next, *prev;
  struct q_class_t *down, *up;
};

struct syntax_t {
  int color;
  int first;
  int last;
};

struct line_t {
  long offset;
  short type;
  short continuation;
  short chunks;
  short search_cnt;
  struct syntax_t *syntax;
  struct syntax_t *search;
  struct q_class_t *quote;
};

#define ANSI_OFF       (1<<0)
#define ANSI_BLINK     (1<<1)
#define ANSI_BOLD      (1<<2)
#define ANSI_UNDERLINE (1<<3)
#define ANSI_REVERSE   (1<<4)
#define ANSI_COLOR     (1<<5)

typedef struct _ansi_attr {
  int attr;
  int fg;
  int bg;
  int pair;
} ansi_attr;

static short InHelp = 0;

#if defined (USE_SLANG_CURSES) || defined (HAVE_RESIZETERM)
static struct resize {
  int line;
  int SearchCompiled;
  int SearchBack;
}     *Resize = NULL;
#endif

#define NumSigLines 4

static int check_sig (const char *s, struct line_t *info, int n)
{
  int count = 0;

  while (n > 0 && count <= NumSigLines) {
    if (info[n].type != MT_COLOR_SIGNATURE)
      break;
    count++;
    n--;
  }

  if (count == 0)
    return (-1);

  if (count > NumSigLines) {
    /* check for a blank line */
    while (*s) {
      if (!ISSPACE (*s))
        return 0;
      s++;
    }

    return (-1);
  }

  return (0);
}

static void
resolve_color (struct line_t *lineInfo, int n, int cnt, int flags,
               int special, ansi_attr * a)
{
  int def_color;                /* color without syntax hilight */
  int color;                    /* final color */
  static int last_color;        /* last color set */
  int search = 0, i, m;

  if (!cnt)
    last_color = -1;            /* force attrset() */

  if (lineInfo[n].continuation) {
    if (!cnt && option (OPTMARKERS)) {
      SETCOLOR (MT_COLOR_MARKERS);
      addch ('+');
      last_color = ColorDefs[MT_COLOR_MARKERS];
    }
    m = (lineInfo[n].syntax)[0].first;
    cnt += (lineInfo[n].syntax)[0].last;
  }
  else
    m = n;
  if (!(flags & M_SHOWCOLOR))
    def_color = ColorDefs[MT_COLOR_NORMAL];
  else if (lineInfo[m].type == MT_COLOR_HEADER)
    def_color = (lineInfo[m].syntax)[0].color;
  else
    def_color = ColorDefs[lineInfo[m].type];

  if ((flags & M_SHOWCOLOR) && lineInfo[m].type == MT_COLOR_QUOTED) {
    struct q_class_t *class = lineInfo[m].quote;

    if (class) {
      def_color = class->color;

      while (class && class->length > cnt) {
        def_color = class->color;
        class = class->up;
      }
    }
  }

  color = def_color;
  if (flags & M_SHOWCOLOR) {
    for (i = 0; i < lineInfo[m].chunks; i++) {
      /* we assume the chunks are sorted */
      if (cnt > (lineInfo[m].syntax)[i].last)
        continue;
      if (cnt < (lineInfo[m].syntax)[i].first)
        break;
      if (cnt != (lineInfo[m].syntax)[i].last) {
        color = (lineInfo[m].syntax)[i].color;
        break;
      }
      /* don't break here, as cnt might be 
       * in the next chunk as well */
    }
  }

  if (flags & M_SEARCH) {
    for (i = 0; i < lineInfo[m].search_cnt; i++) {
      if (cnt > (lineInfo[m].search)[i].last)
        continue;
      if (cnt < (lineInfo[m].search)[i].first)
        break;
      if (cnt != (lineInfo[m].search)[i].last) {
        color = ColorDefs[MT_COLOR_SEARCH];
        search = 1;
        break;
      }
    }
  }

  /* handle "special" bold & underlined characters */
  if (special || a->attr) {
#ifdef HAVE_COLOR
    if ((a->attr & ANSI_COLOR)) {
      if (a->pair == -1)
        a->pair = mutt_alloc_color (a->fg, a->bg);
      color = a->pair;
      if (a->attr & ANSI_BOLD)
        color |= A_BOLD;
    }
    else
#endif
    if ((special & A_BOLD) || (a->attr & ANSI_BOLD)) {
      if (ColorDefs[MT_COLOR_BOLD] && !search)
        color = ColorDefs[MT_COLOR_BOLD];
      else
        color ^= A_BOLD;
    }
    if ((special & A_UNDERLINE) || (a->attr & ANSI_UNDERLINE)) {
      if (ColorDefs[MT_COLOR_UNDERLINE] && !search)
        color = ColorDefs[MT_COLOR_UNDERLINE];
      else
        color ^= A_UNDERLINE;
    }
    else if (a->attr & ANSI_REVERSE) {
      color ^= A_REVERSE;
    }
    else if (a->attr & ANSI_BLINK) {
      color ^= A_BLINK;
    }
    else if (a->attr & ANSI_OFF) {
      a->attr = 0;
    }
  }

  if (color != last_color) {
    attrset (color);
    last_color = color;
  }
}

static void append_line (struct line_t *lineInfo, int n, int cnt)
{
  int m;

  lineInfo[n + 1].type = lineInfo[n].type;
  (lineInfo[n + 1].syntax)[0].color = (lineInfo[n].syntax)[0].color;
  lineInfo[n + 1].continuation = 1;

  /* find the real start of the line */
  for (m = n; m >= 0; m--)
    if (lineInfo[m].continuation == 0)
      break;

  (lineInfo[n + 1].syntax)[0].first = m;
  (lineInfo[n + 1].syntax)[0].last = (lineInfo[n].continuation) ?
    cnt + (lineInfo[n].syntax)[0].last : cnt;
}

static void new_class_color (struct q_class_t *class, int *q_level)
{
  class->index = (*q_level)++;
  class->color = ColorQuote[class->index % ColorQuoteUsed];
}

static void
shift_class_colors (struct q_class_t *QuoteList, struct q_class_t *new_class,
                    int index, int *q_level)
{
  struct q_class_t *q_list;

  q_list = QuoteList;
  new_class->index = -1;

  while (q_list) {
    if (q_list->index >= index) {
      q_list->index++;
      q_list->color = ColorQuote[q_list->index % ColorQuoteUsed];
    }
    if (q_list->down)
      q_list = q_list->down;
    else if (q_list->next)
      q_list = q_list->next;
    else {
      while (!q_list->next) {
        q_list = q_list->up;
        if (q_list == NULL)
          break;
      }
      if (q_list)
        q_list = q_list->next;
    }
  }

  new_class->index = index;
  new_class->color = ColorQuote[index % ColorQuoteUsed];
  (*q_level)++;
}

static void cleanup_quote (struct q_class_t **QuoteList)
{
  struct q_class_t *ptr;

  while (*QuoteList) {
    if ((*QuoteList)->down)
      cleanup_quote (&((*QuoteList)->down));
    ptr = (*QuoteList)->next;
    if ((*QuoteList)->prefix)
      FREE (&(*QuoteList)->prefix);
    FREE (QuoteList);
    *QuoteList = ptr;
  }

  return;
}

static struct q_class_t *classify_quote (struct q_class_t **QuoteList,
                                         const char *qptr, int length,
                                         int *force_redraw, int *q_level)
{
  struct q_class_t *q_list = *QuoteList;
  struct q_class_t *class = NULL, *tmp = NULL, *ptr, *save;
  char *tail_qptr;
  int offset, tail_lng;
  int index = -1;

  if (ColorQuoteUsed <= 1) {
    /* not much point in classifying quotes... */

    if (*QuoteList == NULL) {
      class = (struct q_class_t *) safe_calloc (1, sizeof (struct q_class_t));
      class->color = ColorQuote[0];
      *QuoteList = class;
    }
    return (*QuoteList);
  }

  /* Did I mention how much I like emulating Lisp in C? */

  /* classify quoting prefix */
  while (q_list) {
    if (length <= q_list->length) {
      /* case 1: check the top level nodes */

      if (safe_strncmp (qptr, q_list->prefix, length) == 0) {
        if (length == q_list->length)
          return q_list;        /* same prefix: return the current class */

        /* found shorter prefix */
        if (tmp == NULL) {
          /* add a node above q_list */
          tmp =
            (struct q_class_t *) safe_calloc (1, sizeof (struct q_class_t));
          tmp->prefix = (char *) safe_calloc (1, length + 1);
          strncpy (tmp->prefix, qptr, length);
          tmp->length = length;

          /* replace q_list by tmp in the top level list */
          if (q_list->next) {
            tmp->next = q_list->next;
            q_list->next->prev = tmp;
          }
          if (q_list->prev) {
            tmp->prev = q_list->prev;
            q_list->prev->next = tmp;
          }

          /* make q_list a child of tmp */
          tmp->down = q_list;
          q_list->up = tmp;

          /* q_list has no siblings for now */
          q_list->next = NULL;
          q_list->prev = NULL;

          /* update the root if necessary */
          if (q_list == *QuoteList)
            *QuoteList = tmp;

          index = q_list->index;

          /* tmp should be the return class too */
          class = tmp;

          /* next class to test; if tmp is a shorter prefix for another
           * node, that node can only be in the top level list, so don't
           * go down after this point
           */
          q_list = tmp->next;
        }
        else {
          /* found another branch for which tmp is a shorter prefix */

          /* save the next sibling for later */
          save = q_list->next;

          /* unlink q_list from the top level list */
          if (q_list->next)
            q_list->next->prev = q_list->prev;
          if (q_list->prev)
            q_list->prev->next = q_list->next;

          /* at this point, we have a tmp->down; link q_list to it */
          ptr = tmp->down;
          /* sibling order is important here, q_list should be linked last */
          while (ptr->next)
            ptr = ptr->next;
          ptr->next = q_list;
          q_list->next = NULL;
          q_list->prev = ptr;
          q_list->up = tmp;

          index = q_list->index;

          /* next class to test; as above, we shouldn't go down */
          q_list = save;
        }

        /* we found a shorter prefix, so certain quotes have changed classes */
        *force_redraw = 1;
        continue;
      }
      else {
        /* shorter, but not a substring of the current class: try next */
        q_list = q_list->next;
        continue;
      }
    }
    else {
      /* case 2: try subclassing the current top level node */

      /* tmp != NULL means we already found a shorter prefix at case 1 */
      if (tmp == NULL
          && safe_strncmp (qptr, q_list->prefix, q_list->length) == 0) {
        /* ok, it's a subclass somewhere on this branch */

        ptr = q_list;
        offset = q_list->length;

        q_list = q_list->down;
        tail_lng = length - offset;
        tail_qptr = (char *) qptr + offset;

        while (q_list) {
          if (length <= q_list->length) {
            if (safe_strncmp (tail_qptr, (q_list->prefix) + offset, tail_lng)
                == 0) {
              /* same prefix: return the current class */
              if (length == q_list->length)
                return q_list;

              /* found shorter common prefix */
              if (tmp == NULL) {
                /* add a node above q_list */
                tmp = (struct q_class_t *) safe_calloc (1,
                                                        sizeof (struct
                                                                q_class_t));
                tmp->prefix = (char *) safe_calloc (1, length + 1);
                strncpy (tmp->prefix, qptr, length);
                tmp->length = length;

                /* replace q_list by tmp */
                if (q_list->next) {
                  tmp->next = q_list->next;
                  q_list->next->prev = tmp;
                }
                if (q_list->prev) {
                  tmp->prev = q_list->prev;
                  q_list->prev->next = tmp;
                }

                /* make q_list a child of tmp */
                tmp->down = q_list;
                tmp->up = q_list->up;
                q_list->up = tmp;
                if (tmp->up->down == q_list)
                  tmp->up->down = tmp;

                /* q_list has no siblings */
                q_list->next = NULL;
                q_list->prev = NULL;

                index = q_list->index;

                /* tmp should be the return class too */
                class = tmp;

                /* next class to test */
                q_list = tmp->next;
              }
              else {
                /* found another branch for which tmp is a shorter prefix */

                /* save the next sibling for later */
                save = q_list->next;

                /* unlink q_list from the top level list */
                if (q_list->next)
                  q_list->next->prev = q_list->prev;
                if (q_list->prev)
                  q_list->prev->next = q_list->next;

                /* at this point, we have a tmp->down; link q_list to it */
                ptr = tmp->down;
                while (ptr->next)
                  ptr = ptr->next;
                ptr->next = q_list;
                q_list->next = NULL;
                q_list->prev = ptr;
                q_list->up = tmp;

                index = q_list->index;

                /* next class to test */
                q_list = save;
              }

              /* we found a shorter prefix, so we need a redraw */
              *force_redraw = 1;
              continue;
            }
            else {
              q_list = q_list->next;
              continue;
            }
          }
          else {
            /* longer than the current prefix: try subclassing it */
            if (tmp == NULL
                && safe_strncmp (tail_qptr, (q_list->prefix) + offset,
                                 q_list->length - offset) == 0) {
              /* still a subclass: go down one level */
              ptr = q_list;
              offset = q_list->length;

              q_list = q_list->down;
              tail_lng = length - offset;
              tail_qptr = (char *) qptr + offset;

              continue;
            }
            else {
              /* nope, try the next prefix */
              q_list = q_list->next;
              continue;
            }
          }
        }

        /* still not found so far: add it as a sibling to the current node */
        if (class == NULL) {
          tmp =
            (struct q_class_t *) safe_calloc (1, sizeof (struct q_class_t));
          tmp->prefix = (char *) safe_calloc (1, length + 1);
          strncpy (tmp->prefix, qptr, length);
          tmp->length = length;

          if (ptr->down) {
            tmp->next = ptr->down;
            ptr->down->prev = tmp;
          }
          ptr->down = tmp;
          tmp->up = ptr;

          new_class_color (tmp, q_level);

          return tmp;
        }
        else {
          if (index != -1)
            shift_class_colors (*QuoteList, tmp, index, q_level);

          return class;
        }
      }
      else {
        /* nope, try the next prefix */
        q_list = q_list->next;
        continue;
      }
    }
  }

  if (class == NULL) {
    /* not found so far: add it as a top level class */
    class = (struct q_class_t *) safe_calloc (1, sizeof (struct q_class_t));
    class->prefix = (char *) safe_calloc (1, length + 1);
    strncpy (class->prefix, qptr, length);
    class->length = length;
    new_class_color (class, q_level);

    if (*QuoteList) {
      class->next = *QuoteList;
      (*QuoteList)->prev = class;
    }
    *QuoteList = class;
  }

  if (index != -1)
    shift_class_colors (*QuoteList, tmp, index, q_level);

  return class;
}

static int check_attachment_marker (char *);

static void
resolve_types (char *buf, char *raw, struct line_t *lineInfo, int n, int last,
               struct q_class_t **QuoteList, int *q_level, int *force_redraw,
               int q_classify)
{
  COLOR_LINE *color_line;
  regmatch_t pmatch[1], smatch[1];
  int found, offset, null_rx, i;

  if (n == 0 || ISHEADER (lineInfo[n - 1].type)) {
    if (buf[0] == '\n')
      lineInfo[n].type = MT_COLOR_NORMAL;
    else if (n > 0 && (buf[0] == ' ' || buf[0] == '\t')) {
      lineInfo[n].type = lineInfo[n - 1].type;  /* wrapped line */
      (lineInfo[n].syntax)[0].color = (lineInfo[n - 1].syntax)[0].color;
    }
    else {
      lineInfo[n].type = MT_COLOR_HDEFAULT;
      color_line = ColorHdrList;
      while (color_line) {
        if (REGEXEC (&color_line->rx, buf) == 0) {
          lineInfo[n].type = MT_COLOR_HEADER;
          lineInfo[n].syntax[0].color = color_line->pair;
          break;
        }
        color_line = color_line->next;
      }
    }
  }
  else if (safe_strncmp ("\033[0m", raw, 4) == 0)       /* a little hack... */
    lineInfo[n].type = MT_COLOR_NORMAL;
#if 0
  else if (safe_strncmp ("[-- ", buf, 4) == 0)
    lineInfo[n].type = MT_COLOR_ATTACHMENT;
#else
  else if (check_attachment_marker ((char *) raw) == 0)
    lineInfo[n].type = MT_COLOR_ATTACHMENT;
#endif
  else if (safe_strcmp ("-- \n", buf) == 0
           || safe_strcmp ("-- \r\n", buf) == 0) {
    i = n + 1;

    lineInfo[n].type = MT_COLOR_SIGNATURE;
    while (i < last && check_sig (buf, lineInfo, i - 1) == 0 &&
           (lineInfo[i].type == MT_COLOR_NORMAL ||
            lineInfo[i].type == MT_COLOR_QUOTED ||
            lineInfo[i].type == MT_COLOR_HEADER)) {
      /* oops... */
      if (lineInfo[i].chunks) {
        lineInfo[i].chunks = 0;
        safe_realloc (&(lineInfo[n].syntax), sizeof (struct syntax_t));
      }
      lineInfo[i++].type = MT_COLOR_SIGNATURE;
    }
  }
  else if (check_sig (buf, lineInfo, n - 1) == 0)
    lineInfo[n].type = MT_COLOR_SIGNATURE;
  else if (regexec ((regex_t *) QuoteRegexp.rx, buf, 1, pmatch, 0) == 0) {
    if (regexec ((regex_t *) Smileys.rx, buf, 1, smatch, 0) == 0) {
      if (smatch[0].rm_so > 0) {
        char c;

        /* hack to avoid making an extra copy of buf */
        c = buf[smatch[0].rm_so];
        buf[smatch[0].rm_so] = 0;

        if (regexec ((regex_t *) QuoteRegexp.rx, buf, 1, pmatch, 0) == 0) {
          if (q_classify && lineInfo[n].quote == NULL)
            lineInfo[n].quote = classify_quote (QuoteList,
                                                buf + pmatch[0].rm_so,
                                                pmatch[0].rm_eo -
                                                pmatch[0].rm_so, force_redraw,
                                                q_level);
          lineInfo[n].type = MT_COLOR_QUOTED;
        }
        else
          lineInfo[n].type = MT_COLOR_NORMAL;

        buf[smatch[0].rm_so] = c;
      }
      else
        lineInfo[n].type = MT_COLOR_NORMAL;
    }
    else {
      if (q_classify && lineInfo[n].quote == NULL)
        lineInfo[n].quote = classify_quote (QuoteList, buf + pmatch[0].rm_so,
                                            pmatch[0].rm_eo - pmatch[0].rm_so,
                                            force_redraw, q_level);
      lineInfo[n].type = MT_COLOR_QUOTED;
    }
  }
  else
    lineInfo[n].type = MT_COLOR_NORMAL;

  /* body patterns */
  if (lineInfo[n].type == MT_COLOR_NORMAL ||
      lineInfo[n].type == MT_COLOR_QUOTED) {
    i = 0;

    offset = 0;
    lineInfo[n].chunks = 0;
    do {
      if (!buf[offset])
        break;

      found = 0;
      null_rx = 0;
      color_line = ColorBodyList;
      while (color_line) {
        if (regexec (&color_line->rx, buf + offset, 1, pmatch,
                     (offset ? REG_NOTBOL : 0)) == 0) {
          if (pmatch[0].rm_eo != pmatch[0].rm_so) {
            if (!found) {
              if (++(lineInfo[n].chunks) > 1)
                safe_realloc (&(lineInfo[n].syntax),
                              (lineInfo[n].chunks) *
                              sizeof (struct syntax_t));
            }
            i = lineInfo[n].chunks - 1;
            pmatch[0].rm_so += offset;
            pmatch[0].rm_eo += offset;
            if (!found ||
                pmatch[0].rm_so < (lineInfo[n].syntax)[i].first ||
                (pmatch[0].rm_so == (lineInfo[n].syntax)[i].first &&
                 pmatch[0].rm_eo > (lineInfo[n].syntax)[i].last)) {
              (lineInfo[n].syntax)[i].color = color_line->pair;
              (lineInfo[n].syntax)[i].first = pmatch[0].rm_so;
              (lineInfo[n].syntax)[i].last = pmatch[0].rm_eo;
            }
            found = 1;
            null_rx = 0;
          }
          else
            null_rx = 1;        /* empty regexp; don't add it, but keep looking */
        }
        color_line = color_line->next;
      }

      if (null_rx)
        offset++;               /* avoid degenerate cases */
      else
        offset = (lineInfo[n].syntax)[i].last;
    } while (found || null_rx);
  }
}

static int is_ansi (unsigned char *buf)
{
  while (*buf && (isdigit (*buf) || *buf == ';'))
    buf++;
  return (*buf == 'm');
}

static int check_attachment_marker (char *p)
{
  char *q = AttachmentMarker;

  for (; *p == *q && *q && *p && *q != '\a' && *p != '\a'; p++, q++);
  return (int) (*p - *q);
}

static int grok_ansi (unsigned char *buf, int pos, ansi_attr * a)
{
  int x = pos;

  while (isdigit (buf[x]) || buf[x] == ';')
    x++;

  /* Character Attributes */
  if (option (OPTALLOWANSI) && a != NULL && buf[x] == 'm') {
    if (pos == x) {
#ifdef HAVE_COLOR
      if (a->pair != -1)
        mutt_free_color (a->fg, a->bg);
#endif
      a->attr = ANSI_OFF;
      a->pair = -1;
    }
    while (pos < x) {
      if (buf[pos] == '1' && (pos + 1 == x || buf[pos + 1] == ';')) {
        a->attr |= ANSI_BOLD;
        pos += 2;
      }
      else if (buf[pos] == '4' && (pos + 1 == x || buf[pos + 1] == ';')) {
        a->attr |= ANSI_UNDERLINE;
        pos += 2;
      }
      else if (buf[pos] == '5' && (pos + 1 == x || buf[pos + 1] == ';')) {
        a->attr |= ANSI_BLINK;
        pos += 2;
      }
      else if (buf[pos] == '7' && (pos + 1 == x || buf[pos + 1] == ';')) {
        a->attr |= ANSI_REVERSE;
        pos += 2;
      }
      else if (buf[pos] == '0' && (pos + 1 == x || buf[pos + 1] == ';')) {
#ifdef HAVE_COLOR
        if (a->pair != -1)
          mutt_free_color (a->fg, a->bg);
#endif
        a->attr = ANSI_OFF;
        a->pair = -1;
        pos += 2;
      }
      else if (buf[pos] == '3' && isdigit (buf[pos + 1])) {
#ifdef HAVE_COLOR
        if (a->pair != -1)
          mutt_free_color (a->fg, a->bg);
#endif
        a->pair = -1;
        a->attr |= ANSI_COLOR;
        a->fg = buf[pos + 1] - '0';
        pos += 3;
      }
      else if (buf[pos] == '4' && isdigit (buf[pos + 1])) {
#ifdef HAVE_COLOR
        if (a->pair != -1)
          mutt_free_color (a->fg, a->bg);
#endif
        a->pair = -1;
        a->attr |= ANSI_COLOR;
        a->bg = buf[pos + 1] - '0';
        pos += 3;
      }
      else {
        while (pos < x && buf[pos] != ';')
          pos++;
        pos++;
      }
    }
  }
  pos = x;
  return pos;
}

static int
fill_buffer (FILE * f, long *last_pos, long offset, unsigned char *buf,
             unsigned char *fmt, size_t blen, int *buf_ready)
{
  unsigned char *p;
  static int b_read = 0;

  if (*buf_ready == 0) {
    buf[blen - 1] = 0;
    if (offset != *last_pos)
      fseek (f, offset, 0);
    if (fgets ((char *) buf, blen - 1, f) == NULL) {
      fmt[0] = 0;
      return (-1);
    }
    *last_pos = ftell (f);
    b_read = (int) (*last_pos - offset);
    *buf_ready = 1;

    /* copy "buf" to "fmt", but without bold and underline controls */
    p = buf;
    while (*p) {
      if (*p == '\010' && (p > buf)) {
        if (*(p + 1) == '_')    /* underline */
          p += 2;
        else if (*(p + 1)) {    /* bold or overstrike */
          *(fmt - 1) = *(p + 1);
          p += 2;
        }
        else                    /* ^H */
          *fmt++ = *p++;
      }
      else if (*p == '\033' && *(p + 1) == '[' && is_ansi (p + 2)) {
        while (*p++ != 'm')     /* skip ANSI sequence */
          ;
      }
      else if (*p == '\033' && *(p + 1) == ']'
               && check_attachment_marker ((char *) p) == 0) {
        debug_print (2, ("seen attachment marker.\n"));
        while (*p++ != '\a')    /* skip pseudo-ANSI sequence */
          ;
      }
      else
        *fmt++ = *p++;
    }
    *fmt = 0;
  }
  return b_read;
}

#ifdef USE_NNTP
#include "mx.h"
#include "nntp.h"
#endif


static int format_line (struct line_t **lineInfo, int n, unsigned char *buf,
                        int flags, ansi_attr * pa, int cnt,
                        int *pspace, int *pvch, int *pcol, int *pspecial)
{
  int space = -1;               /* index of the last space or TAB */
  int col = option (OPTMARKERS) ? (*lineInfo)[n].continuation : 0;
  int ch, vch, k, last_special = -1, special = 0, t;
  wchar_t wc;
  mbstate_t mbstate;

  int wrap_cols = COLS;

  if (!(flags & (M_SHOWFLAT)))
    wrap_cols -= WrapMargin;
  wrap_cols -= SidebarWidth;

  if (wrap_cols <= 0)
    wrap_cols = COLS;

  /* FIXME: this should come from lineInfo */
  memset (&mbstate, 0, sizeof (mbstate));

  for (ch = 0, vch = 0; ch < cnt; ch += k, vch += k) {
    /* Handle ANSI sequences */
    while (cnt - ch >= 2 && buf[ch] == '\033' && buf[ch + 1] == '[' &&
           is_ansi (buf + ch + 2))
      ch = grok_ansi (buf, ch + 2, pa) + 1;

    while (cnt - ch >= 2 && buf[ch] == '\033' && buf[ch + 1] == ']' &&
           check_attachment_marker ((char *) buf + ch) == 0) {
      while (buf[ch++] != '\a')
        if (ch >= cnt)
          break;
    }

    /* is anything left to do? */
    if (ch >= cnt)
      break;

    k = mbrtowc (&wc, (char *) buf + ch, cnt - ch, &mbstate);
    if (k == -2 || k == -1) {
      debug_print (1, ("mbrtowc returned %d; errno = %d.\n", k, errno));
      if (col + 4 > wrap_cols)
        break;
      col += 4;
      if (pa)
        printw ("\\%03o", buf[ch]);
      k = 1;
      continue;
    }
    if (k == 0)
      k = 1;

    /* Handle backspace */
    special = 0;
    if (IsWPrint (wc)) {
      wchar_t wc1;
      mbstate_t mbstate1;
      int k1, k2;

      while ((wc1 = 0, mbstate1 = mbstate,
              k1 =
              k + mbrtowc (&wc1, (char *) buf + ch + k, cnt - ch - k,
                           &mbstate1), k1 - k > 0 && wc1 == '\b')
             && (wc1 = 0, k2 =
                 mbrtowc (&wc1, (char *) buf + ch + k1, cnt - ch - k1,
                          &mbstate1), k2 > 0 && IsWPrint (wc1))) {
        if (wc == wc1) {
          special |= (wc == '_' && special & A_UNDERLINE)
            ? A_UNDERLINE : A_BOLD;
        }
        else if (wc == '_' || wc1 == '_') {
          special |= A_UNDERLINE;
          wc = (wc1 == '_') ? wc : wc1;
        }
        else {
          /* special = 0; / * overstrike: nothing to do! */
          wc = wc1;
        }
        ch += k1;
        k = k2;
        mbstate = mbstate1;
      }
    }

    if (pa &&
        ((flags & (M_SHOWCOLOR | M_SEARCH | M_PAGER_MARKER)) ||
         special || last_special || pa->attr)) {
      resolve_color (*lineInfo, n, vch, flags, special, pa);
      last_special = special;
    }

    if (IsWPrint (wc)) {
      if (wc == ' ')
        space = ch;
      t = wcwidth (wc);
      if (col + t > wrap_cols)
        break;
      col += t;
      if (pa)
        mutt_addwch (wc);
    }
    else if (wc == '\n')
      break;
    else if (wc == '\t') {
      space = ch;
      t = (col & ~7) + 8;
      if (t > wrap_cols)
        break;
      if (pa)
        for (; col < t; col++)
          addch (' ');
      else
        col = t;
    }
    else if (wc < 0x20 || wc == 0x7f) {
      if (col + 2 > wrap_cols)
        break;
      col += 2;
      if (pa)
        printw ("^%c", ('@' + wc) & 0x7f);
    }
    else if (wc < 0x100) {
      if (col + 4 > wrap_cols)
        break;
      col += 4;
      if (pa)
        printw ("\\%03o", wc);
    }
    else {
      if (col + 1 > wrap_cols)
        break;
      ++col;
      if (pa)
        addch (replacement_char ());
    }
  }
  *pspace = space;
  *pcol = col;
  *pvch = vch;
  *pspecial = special;
  return ch;
}

/*
 * Args:
 *	flags	M_SHOWFLAT, show characters (used for displaying help)
 *		M_SHOWCOLOR, show characters in color
 *			otherwise don't show characters
 *		M_HIDE, don't show quoted text
 *		M_SEARCH, resolve search patterns
 *		M_TYPES, compute line's type
 *		M_PAGER_NSKIP, keeps leading whitespace
 *		M_PAGER_MARKER, eventually show markers
 *
 * Return values:
 *	-1	EOF was reached
 *	0	normal exit, line was not displayed
 *	>0	normal exit, line was displayed
 */

static int
display_line (FILE * f, long *last_pos, struct line_t **lineInfo, int n,
              int *last, int *max, int flags, struct q_class_t **QuoteList,
              int *q_level, int *force_redraw, regex_t * SearchRE)
{
  unsigned char buf[LONG_STRING], fmt[LONG_STRING];
  unsigned char *buf_ptr = buf;
  int ch, vch, col, cnt, b_read;
  int buf_ready = 0, change_last = 0;
  int special;
  int offset;
  int def_color;
  int m;
  ansi_attr a = { 0, 0, 0, -1 };
  regmatch_t pmatch[1];

  if (n == *last) {
    (*last)++;
    change_last = 1;
  }

  if (*last == *max) {
    safe_realloc (lineInfo, sizeof (struct line_t) * (*max += LINES));
    for (ch = *last; ch < *max; ch++) {
      memset (&((*lineInfo)[ch]), 0, sizeof (struct line_t));
      (*lineInfo)[ch].type = -1;
      (*lineInfo)[ch].search_cnt = -1;
      (*lineInfo)[ch].syntax = safe_malloc (sizeof (struct syntax_t));
      ((*lineInfo)[ch].syntax)[0].first = ((*lineInfo)[ch].syntax)[0].last =
        -1;
    }
  }

  /* only do color hiliting if we are viewing a message */
  if (flags & (M_SHOWCOLOR | M_TYPES)) {
    if ((*lineInfo)[n].type == -1) {
      /* determine the line class */
      if (fill_buffer
          (f, last_pos, (*lineInfo)[n].offset, buf, fmt, sizeof (buf),
           &buf_ready) < 0) {
        if (change_last)
          (*last)--;
        return (-1);
      }

      resolve_types ((char *) fmt, (char *) buf, *lineInfo, n, *last,
                     QuoteList, q_level, force_redraw, flags & M_SHOWCOLOR);

      /* avoid race condition for continuation lines when scrolling up */
      for (m = n + 1;
           m < *last && (*lineInfo)[m].offset && (*lineInfo)[m].continuation;
           m++)
        (*lineInfo)[m].type = (*lineInfo)[n].type;
    }

    /* this also prevents searching through the hidden lines */
    if ((flags & M_HIDE) && (*lineInfo)[n].type == MT_COLOR_QUOTED)
      flags = 0;                /* M_NOSHOW */
  }

  /* At this point, (*lineInfo[n]).quote may still be undefined. We 
   * don't want to compute it every time M_TYPES is set, since this
   * would slow down the "bottom" function unacceptably. A compromise
   * solution is hence to call regexec() again, just to find out the
   * length of the quote prefix.
   */
  if ((flags & M_SHOWCOLOR) && !(*lineInfo)[n].continuation &&
      (*lineInfo)[n].type == MT_COLOR_QUOTED && (*lineInfo)[n].quote == NULL)
  {
    if (fill_buffer
        (f, last_pos, (*lineInfo)[n].offset, buf, fmt, sizeof (buf),
         &buf_ready) < 0) {
      if (change_last)
        (*last)--;
      return (-1);
    }
    regexec ((regex_t *) QuoteRegexp.rx, (char *) fmt, 1, pmatch, 0);
    (*lineInfo)[n].quote = classify_quote (QuoteList,
                                           (char *) fmt + pmatch[0].rm_so,
                                           pmatch[0].rm_eo - pmatch[0].rm_so,
                                           force_redraw, q_level);
  }

  if ((flags & M_SEARCH) && !(*lineInfo)[n].continuation
      && (*lineInfo)[n].search_cnt == -1) {
    if (fill_buffer
        (f, last_pos, (*lineInfo)[n].offset, buf, fmt, sizeof (buf),
         &buf_ready) < 0) {
      if (change_last)
        (*last)--;
      return (-1);
    }

    offset = 0;
    (*lineInfo)[n].search_cnt = 0;
    while (regexec
           (SearchRE, (char *) fmt + offset, 1, pmatch,
            (offset ? REG_NOTBOL : 0)) == 0) {
      if (++((*lineInfo)[n].search_cnt) > 1)
        safe_realloc (&((*lineInfo)[n].search),
                      ((*lineInfo)[n].search_cnt) * sizeof (struct syntax_t));
      else
        (*lineInfo)[n].search = safe_malloc (sizeof (struct syntax_t));
      pmatch[0].rm_so += offset;
      pmatch[0].rm_eo += offset;
      ((*lineInfo)[n].search)[(*lineInfo)[n].search_cnt - 1].first =
        pmatch[0].rm_so;
      ((*lineInfo)[n].search)[(*lineInfo)[n].search_cnt - 1].last =
        pmatch[0].rm_eo;

      if (pmatch[0].rm_eo == pmatch[0].rm_so)
        offset++;               /* avoid degenerate cases */
      else
        offset = pmatch[0].rm_eo;
      if (!fmt[offset])
        break;
    }
  }

  if (!(flags & M_SHOW) && (*lineInfo)[n + 1].offset > 0) {
    /* we've already scanned this line, so just exit */
    return (0);
  }
  if ((flags & M_SHOWCOLOR) && *force_redraw && (*lineInfo)[n + 1].offset > 0) {
    /* no need to try to display this line... */
    return (1);                 /* fake display */
  }

  if ((b_read = fill_buffer (f, last_pos, (*lineInfo)[n].offset, buf, fmt,
                             sizeof (buf), &buf_ready)) < 0) {
    if (change_last)
      (*last)--;
    return (-1);
  }

  /* now chose a good place to break the line */
  cnt =
    format_line (lineInfo, n, buf, flags, 0, b_read, &ch, &vch, &col,
                 &special);
  buf_ptr = buf + cnt;

  /* move the break point only if smart_wrap is set */
  if (option (OPTWRAP)) {
    if (cnt < b_read) {
      if (ch != -1 && buf[cnt] != ' ' && buf[cnt] != '\t' && buf[cnt] != '\n'
          && buf[cnt] != '\r') {
        buf_ptr = buf + ch;
        /* skip trailing blanks */
        while (ch && (buf[ch] == ' ' || buf[ch] == '\t' || buf[ch] == '\r'))
          ch--;
        cnt = ch + 1;
      }
      else
        buf_ptr = buf + cnt;    /* a very long word... */
    }
    if (!(flags & M_PAGER_NSKIP))
      /* skip leading blanks on the next line too */
      while (*buf_ptr == ' ' || *buf_ptr == '\t')
        buf_ptr++;
  }

  if (*buf_ptr == '\r')
    buf_ptr++;
  if (*buf_ptr == '\n')
    buf_ptr++;

  if ((int) (buf_ptr - buf) < b_read && !(*lineInfo)[n + 1].continuation)
    append_line (*lineInfo, n, (int) (buf_ptr - buf));
  (*lineInfo)[n + 1].offset = (*lineInfo)[n].offset + (long) (buf_ptr - buf);

  /* if we don't need to display the line we are done */
  if (!(flags & M_SHOW))
    return 0;

  /* display the line */
  format_line (lineInfo, n, buf, flags, &a, cnt, &ch, &vch, &col, &special);

  /* avoid a bug in ncurses... */
#ifndef USE_SLANG_CURSES
  if (col == 0) {
    SETCOLOR (MT_COLOR_NORMAL);
    addch (' ');
  }
#endif

  /* end the last color pattern (needed by S-Lang) */
  if (special || (col != COLS && (flags & (M_SHOWCOLOR | M_SEARCH))))
    resolve_color (*lineInfo, n, vch, flags, 0, &a);

  /*
   * Fill the blank space at the end of the line with the prevailing color.
   * ncurses does an implicit clrtoeol() when you do addch('\n') so we have
   * to make sure to reset the color *after* that
   */
  if (flags & M_SHOWCOLOR) {
    m = ((*lineInfo)[n].continuation) ? ((*lineInfo)[n].syntax)[0].first : n;
    if ((*lineInfo)[m].type == MT_COLOR_HEADER)
      def_color = ((*lineInfo)[m].syntax)[0].color;
    else
      def_color = ColorDefs[(*lineInfo)[m].type];

    attrset (def_color);
#ifdef HAVE_BKGDSET
    bkgdset (def_color | ' ');
#endif
  }

  /* ncurses always wraps lines when you get to the right side of the
   * screen, but S-Lang seems to only wrap if the next character is *not*
   * a newline (grr!).
   */
#ifndef USE_SLANG_CURSES
  if (col < COLS)
#endif
    addch ('\n');

  /*
   * reset the color back to normal.  This *must* come after the
   * addch('\n'), otherwise the color for this line will not be
   * filled to the right margin.
   */
  if (flags & M_SHOWCOLOR) {
    SETCOLOR (MT_COLOR_NORMAL);
    BKGDSET (MT_COLOR_NORMAL);
  }

  /* build a return code */
  if (!(flags & M_SHOW))
    flags = 0;

  return (flags);
}

static int upNLines (int nlines, struct line_t *info, int cur, int hiding)
{
  while (cur > 0 && nlines > 0) {
    cur--;
    if (!hiding || info[cur].type != MT_COLOR_QUOTED)
      nlines--;
  }

  return cur;
}

static struct mapping_t PagerHelp[] = {
  {N_("Exit"), OP_EXIT},
  {N_("PrevPg"), OP_PREV_PAGE},
  {N_("NextPg"), OP_NEXT_PAGE},
  {NULL, 0}
};
static struct mapping_t PagerHelpExtra[] = {
  {N_("View Attachm."), OP_VIEW_ATTACHMENTS},
  {N_("Del"), OP_DELETE},
  {N_("Reply"), OP_REPLY},
  {N_("Next"), OP_MAIN_NEXT_UNDELETED},
  {NULL, 0}
};

#ifdef USE_NNTP
static struct mapping_t PagerNewsHelpExtra[] = {
  {N_("Post"), OP_POST},
  {N_("Followup"), OP_FOLLOWUP},
  {N_("Del"), OP_DELETE},
  {N_("Next"), OP_MAIN_NEXT_UNDELETED},
  {NULL, 0}
};
#endif



/* This pager is actually not so simple as it once was.  It now operates in
   two modes: one for viewing messages and the other for viewing help.  These
   can be distinguished by whether or not ``hdr'' is NULL.  The ``hdr'' arg
   is there so that we can do operations on the current message without the
   need to pop back out to the main-menu.  */
int
mutt_pager (const char *banner, const char *fname, int flags, pager_t * extra)
{
  static char searchbuf[STRING];
  char buffer[LONG_STRING];
  char helpstr[SHORT_STRING * 2];
  char tmphelp[SHORT_STRING * 2];
  int maxLine, lastLine = 0;
  struct line_t *lineInfo;
  struct q_class_t *QuoteList = NULL;
  int i, j, ch = 0, rc = -1, hideQuoted = 0, q_level = 0, force_redraw = 0;
  int lines = 0, curline = 0, topline = 0, oldtopline = 0, err, first = 1;
  int r = -1;
  int redraw = REDRAW_FULL;
  FILE *fp = NULL;
  long last_pos = 0, last_offset = 0;
  int old_smart_wrap, old_markers;
  struct stat sb;
  regex_t SearchRE;
  int SearchCompiled = 0, SearchFlag = 0, SearchBack = 0;
  int has_types = (IsHeader (extra) || (flags & M_SHOWCOLOR)) ? M_TYPES : 0;    /* main message or rfc822 attachment */

  int bodyoffset = 1;           /* offset of first line of real text */
  int statusoffset = 0;         /* offset for the status bar */
  int helpoffset = LINES - 2;   /* offset for the help bar. */
  int bodylen = LINES - 2 - bodyoffset; /* length of displayable area */

  MUTTMENU *index = NULL;       /* the Pager Index (PI) */
  int indexoffset = 0;          /* offset for the PI */
  int indexlen = PagerIndexLines;       /* indexlen not always == PIL */
  int indicator = indexlen / 3; /* the indicator line of the PI */
  int old_PagerIndexLines;      /* some people want to resize it
                                 * while inside the pager... */

#ifdef USE_NNTP
  char *followup_to;
#endif

  if (!(flags & M_SHOWCOLOR))
    flags |= M_SHOWFLAT;

  if ((fp = fopen (fname, "r")) == NULL) {
    mutt_perror (fname);
    return (-1);
  }

  if (stat (fname, &sb) != 0) {
    mutt_perror (fname);
    fclose (fp);
    return (-1);
  }
  unlink (fname);

  /* Initialize variables */

  if (IsHeader (extra) && !extra->hdr->read) {
    Context->msgnotreadyet = extra->hdr->msgno;
    mutt_set_flag (Context, extra->hdr, M_READ, 1);
  }

  lineInfo = safe_malloc (sizeof (struct line_t) * (maxLine = LINES));
  for (i = 0; i < maxLine; i++) {
    memset (&lineInfo[i], 0, sizeof (struct line_t));
    lineInfo[i].type = -1;
    lineInfo[i].search_cnt = -1;
    lineInfo[i].syntax = safe_malloc (sizeof (struct syntax_t));
    (lineInfo[i].syntax)[0].first = (lineInfo[i].syntax)[0].last = -1;
  }

  mutt_compile_help (helpstr, sizeof (helpstr), MENU_PAGER, PagerHelp);
  if (IsHeader (extra)) {
    strfcpy (tmphelp, helpstr, sizeof (tmphelp));
    mutt_compile_help (buffer, sizeof (buffer), MENU_PAGER,
#ifdef USE_NNTP
                       (Context
                        && (Context->magic == M_NNTP)) ? PagerNewsHelpExtra :
#endif
                       PagerHelpExtra);
    snprintf (helpstr, sizeof (helpstr), "%s %s", tmphelp, buffer);
  }
  if (!InHelp) {
    strfcpy (tmphelp, helpstr, sizeof (tmphelp));
    mutt_make_help (buffer, sizeof (buffer), _("Help"), MENU_PAGER, OP_HELP);
    snprintf (helpstr, sizeof (helpstr), "%s %s", tmphelp, buffer);
  }

  while (ch != -1) {
    mutt_curs_set (0);

#ifdef USE_IMAP
    imap_keepalive ();
#endif

    if (redraw & REDRAW_FULL) {
      SETCOLOR (MT_COLOR_NORMAL);
      /* clear() doesn't optimize screen redraws */
      move (0, 0);
      clrtobot ();

      if (IsHeader (extra) && Context->vcount + 1 < PagerIndexLines)
        indexlen = Context->vcount + 1;
      else
        indexlen = PagerIndexLines;

      indicator = indexlen / 3;

      if (option (OPTSTATUSONTOP)) {
        indexoffset = 0;
        statusoffset = IsHeader (extra) ? indexlen : 0;
        bodyoffset = statusoffset + 1;
        helpoffset = LINES - 2;
        bodylen = helpoffset - bodyoffset;
        if (!option (OPTHELP))
          bodylen++;
      }
      else {
        helpoffset = 0;
        indexoffset = 1;
        statusoffset = LINES - 2;
        if (!option (OPTHELP))
          indexoffset = 0;
        bodyoffset = indexoffset + (IsHeader (extra) ? indexlen : 0);
        bodylen = statusoffset - bodyoffset;
      }

      if (option (OPTHELP)) {
        SETCOLOR (MT_COLOR_STATUS);
        move (helpoffset, 0);
        mutt_paddstr (COLS, helpstr);
        SETCOLOR (MT_COLOR_NORMAL);
      }

#if defined (USE_SLANG_CURSES) || defined (HAVE_RESIZETERM)
      if (Resize != NULL) {
        if ((SearchCompiled = Resize->SearchCompiled)) {
          REGCOMP
            (&SearchRE, searchbuf, REG_NEWLINE | mutt_which_case (searchbuf));
          SearchFlag = M_SEARCH;
          SearchBack = Resize->SearchBack;
        }
        lines = Resize->line;
        redraw |= REDRAW_SIGWINCH;

        FREE (&Resize);
      }
#endif

      if (IsHeader (extra) && PagerIndexLines) {
        if (index == NULL) {
          /* only allocate the space if/when we need the index.
             Initialise the menu as per the main index */
          index = mutt_new_menu ();
          index->menu = MENU_MAIN;
          index->make_entry = index_make_entry;
          index->color = index_color;
          index->max = Context->vcount;
          index->current = extra->hdr->virtual;
        }

        SETCOLOR (MT_COLOR_NORMAL);
        index->offset = indexoffset + (option (OPTSTATUSONTOP) ? 1 : 0);

        index->pagelen = indexlen - 1;

        /* some fudge to work out where abouts the indicator should go */
        if (index->current - indicator < 0)
          index->top = 0;
        else if (index->max - index->current < index->pagelen - indicator)
          index->top = index->max - index->pagelen;
        else
          index->top = index->current - indicator;

        menu_redraw_index (index);
      }

      redraw |= REDRAW_BODY | REDRAW_INDEX | REDRAW_STATUS;
      mutt_show_error ();
    }

    if (redraw & REDRAW_SIGWINCH) {
      i = -1;
      j = -1;
      while (display_line (fp, &last_pos, &lineInfo, ++i, &lastLine, &maxLine,
                           has_types | SearchFlag, &QuoteList, &q_level,
                           &force_redraw, &SearchRE) == 0) {
        if (!lineInfo[i].continuation && ++j == lines) {
          topline = i;
          if (!SearchFlag)
            break;
        }
        redraw |= REDRAW_SIDEBAR;
      }                         /* while */
    }

    if ((redraw & REDRAW_BODY) || topline != oldtopline) {
      do {
        move (bodyoffset, SidebarWidth);
        curline = oldtopline = topline;
        lines = 0;
        force_redraw = 0;

        while (lines < bodylen && lineInfo[curline].offset <= sb.st_size - 1) {
          if (display_line (fp, &last_pos, &lineInfo, curline, &lastLine,
                            &maxLine,
                            (flags & M_DISPLAYFLAGS) | hideQuoted |
                            SearchFlag, &QuoteList, &q_level, &force_redraw,
                            &SearchRE) > 0)
            lines++;
          curline++;
          move (lines + bodyoffset, SidebarWidth);
          redraw |= REDRAW_SIDEBAR;
        }
        last_offset = lineInfo[curline].offset;
      } while (force_redraw);

      SETCOLOR (MT_COLOR_TILDE);
      BKGDSET (MT_COLOR_TILDE);
      while (lines < bodylen) {
        clrtoeol ();
        if (option (OPTTILDE))
          addch ('~');
        addch ('\n');
        lines++;
        move (lines + bodyoffset, SidebarWidth);
      }
      /* We are going to update the pager status bar, so it isn't
       * necessary to reset to normal color now. */

      redraw |= REDRAW_STATUS;  /* need to update the % seen */
    }

    if (redraw & REDRAW_STATUS) {
      /* print out the pager status bar */
      SETCOLOR (MT_COLOR_STATUS);
      BKGDSET (MT_COLOR_STATUS);
      CLEARLINE_WIN (statusoffset);
      if (IsHeader (extra)) {
        size_t l1 = (COLS - 9) * MB_LEN_MAX;
        size_t l2 = sizeof (buffer);

        _mutt_make_string (buffer, l1 < l2 ? l1 : l2, NONULL (PagerFmt),
                           Context, extra->hdr, M_FORMAT_MAKEPRINT);
      }
      else if (IsMsgAttach (extra)) {
        size_t l1 = (COLS - 9) * MB_LEN_MAX;
        size_t l2 = sizeof (buffer);

        _mutt_make_string (buffer, l1 < l2 ? l1 : l2, NONULL (PagerFmt),
                           Context, extra->bdy->hdr, M_FORMAT_MAKEPRINT);
      }
      if (option(OPTSTATUSONTOP)) {
        move(0,0);
      }
      /*move (indexoffset + (option (OPTSTATUSONTOP) ? 0 : (InHelp?(LINES-2):(indexlen - 1))),
            option (OPTSTATUSONTOP) ? 0 : SidebarWidth);*/
      mutt_paddstr (COLS - 10 - (option(OPTSTATUSONTOP)?0:SidebarWidth), IsHeader (extra)
                    || IsMsgAttach (extra) ? buffer : banner);

      addstr (" -- (");
      if (last_pos < sb.st_size - 1)
        printw ("%d%%)", (int) (100 * last_offset / sb.st_size));
      else
        addstr (topline == 0 ? "all)" : "end)");
      BKGDSET (MT_COLOR_NORMAL);
      SETCOLOR (MT_COLOR_NORMAL);
    }

    if (redraw & REDRAW_SIDEBAR)
      sidebar_draw (MENU_PAGER);

    if ((redraw & REDRAW_INDEX) && index) {
      /* redraw the pager_index indicator, because the
       * flags for this message might have changed. */
      menu_redraw_current (index);
      sidebar_draw (MENU_PAGER);
      /* print out the index status bar */
      menu_status_line (buffer, sizeof (buffer), index, NONULL (Status));
      move (indexoffset + (option (OPTSTATUSONTOP) ? 0 : (indexlen - 1)),
            option (OPTSTATUSONTOP) ? 0 : SidebarWidth);
      SETCOLOR (MT_COLOR_STATUS);
      mutt_paddstr (COLS - (option (OPTSTATUSONTOP) ? 0 : SidebarWidth),
                    buffer);
      SETCOLOR (MT_COLOR_NORMAL);
    }
    /* if we're not using the index, update every time */
    if (index == 0)
      sidebar_draw (MENU_PAGER);

    redraw = 0;

    move (statusoffset, COLS - 1);
    mutt_refresh ();
    ch = km_dokey (MENU_PAGER);
    if (ch != -1)
      mutt_clear_error ();
    mutt_curs_set (1);

    if (SigInt) {
      mutt_query_exit ();
      continue;
    }
#if defined (USE_SLANG_CURSES) || defined (HAVE_RESIZETERM)
    else if (SigWinch) {
      mutt_resize_screen ();

      /* Store current position. */
      lines = -1;
      for (i = 0; i <= topline; i++)
        if (!lineInfo[i].continuation)
          lines++;

      if (flags & M_PAGER_RETWINCH) {
        Resize = safe_malloc (sizeof (struct resize));

        Resize->line = lines;
        Resize->SearchCompiled = SearchCompiled;
        Resize->SearchBack = SearchBack;

        ch = -1;
        rc = OP_REFORMAT_WINCH;
      }
      else {
        for (i = 0; i < maxLine; i++) {
          lineInfo[i].offset = 0;
          lineInfo[i].type = -1;
          lineInfo[i].continuation = 0;
          lineInfo[i].chunks = 0;
          lineInfo[i].search_cnt = -1;
          lineInfo[i].quote = NULL;

          safe_realloc (&(lineInfo[i].syntax), sizeof (struct syntax_t));
          if (SearchCompiled && lineInfo[i].search)
            FREE (&(lineInfo[i].search));
        }

        lastLine = 0;
        topline = 0;

        redraw = REDRAW_FULL | REDRAW_SIGWINCH;
        ch = 0;
      }

      SigWinch = 0;
      clearok (stdscr, TRUE);   /*force complete redraw */
      continue;
    }
#endif
    else if (ch == -1) {
      ch = 0;
      continue;
    }

    rc = ch;

    switch (ch) {
    case OP_EXIT:
      rc = -1;
      ch = -1;
      break;

    case OP_NEXT_PAGE:
      if (lineInfo[curline].offset < sb.st_size - 1) {
        topline = upNLines (PagerContext, lineInfo, curline, hideQuoted);
      }
      else if (option (OPTPAGERSTOP)) {
        /* emulate "less -q" and don't go on to the next message. */
        mutt_error _("Bottom of message is shown.");
      }
      else {
        /* end of the current message, so display the next message. */
        rc = OP_MAIN_NEXT_UNDELETED;
        ch = -1;
      }
      break;

    case OP_PREV_PAGE:
      if (topline != 0) {
        topline =
          upNLines (bodylen - PagerContext, lineInfo, topline, hideQuoted);
      }
      else
        mutt_error _("Top of message is shown.");
      break;

    case OP_NEXT_LINE:
      if (lineInfo[curline].offset < sb.st_size - 1) {
        topline++;
        if (hideQuoted) {
          while (lineInfo[topline].type == MT_COLOR_QUOTED &&
                 topline < lastLine)
            topline++;
        }
      }
      else
        mutt_error _("Bottom of message is shown.");
      break;

    case OP_PREV_LINE:
      if (topline)
        topline = upNLines (1, lineInfo, topline, hideQuoted);
      else
        mutt_error _("Top of message is shown.");
      break;

    case OP_PAGER_TOP:
      if (topline)
        topline = 0;
      else
        mutt_error _("Top of message is shown.");
      break;

    case OP_HALF_UP:
      if (topline)
        topline = upNLines (bodylen / 2, lineInfo, topline, hideQuoted);
      else
        mutt_error _("Top of message is shown.");
      break;

    case OP_HALF_DOWN:
      if (lineInfo[curline].offset < sb.st_size - 1) {
        topline = upNLines (bodylen / 2, lineInfo, curline, hideQuoted);
      }
      else if (option (OPTPAGERSTOP)) {
        /* emulate "less -q" and don't go on to the next message. */
        mutt_error _("Bottom of message is shown.");
      }
      else {
        /* end of the current message, so display the next message. */
        rc = OP_MAIN_NEXT_UNDELETED;
        ch = -1;
      }
      break;

    case OP_SEARCH_NEXT:
    case OP_SEARCH_OPPOSITE:
      if (SearchCompiled) {
      search_next:
        if ((!SearchBack && ch == OP_SEARCH_NEXT) ||
            (SearchBack && ch == OP_SEARCH_OPPOSITE)) {
          /* searching forward */
          for (i = topline + 1; i < lastLine; i++) {
            if ((!hideQuoted || lineInfo[i].type != MT_COLOR_QUOTED) &&
                !lineInfo[i].continuation && lineInfo[i].search_cnt > 0)
              break;
          }

          if (i < lastLine)
            topline = i;
          else
            mutt_error _("Not found.");
        }
        else {
          /* searching backward */
          for (i = topline - 1; i >= 0; i--) {
            if ((!hideQuoted || (has_types &&
                                 lineInfo[i].type != MT_COLOR_QUOTED)) &&
                !lineInfo[i].continuation && lineInfo[i].search_cnt > 0)
              break;
          }

          if (i >= 0)
            topline = i;
          else
            mutt_error _("Not found.");
        }

        if (lineInfo[topline].search_cnt > 0)
          SearchFlag = M_SEARCH;

        break;
      }
      /* no previous search pattern, so fall through to search */

    case OP_SEARCH:
    case OP_SEARCH_REVERSE:
      strfcpy (buffer, searchbuf, sizeof (buffer));
      if (mutt_get_field ((SearchBack ? _("Reverse search: ") :
                           _("Search: ")), buffer, sizeof (buffer),
                          M_CLEAR) != 0)
        break;

      if (!strcmp (buffer, searchbuf)) {
        if (SearchCompiled) {
          /* do an implicit search-next */
          if (ch == OP_SEARCH)
            ch = OP_SEARCH_NEXT;
          else
            ch = OP_SEARCH_OPPOSITE;

          goto search_next;
        }
      }

      if (!buffer[0])
        break;

      strfcpy (searchbuf, buffer, sizeof (searchbuf));

      /* leave SearchBack alone if ch == OP_SEARCH_NEXT */
      if (ch == OP_SEARCH)
        SearchBack = 0;
      else if (ch == OP_SEARCH_REVERSE)
        SearchBack = 1;

      if (SearchCompiled) {
        regfree (&SearchRE);
        for (i = 0; i < lastLine; i++) {
          if (lineInfo[i].search)
            FREE (&(lineInfo[i].search));
          lineInfo[i].search_cnt = -1;
        }
      }

      if ((err =
           REGCOMP (&SearchRE, searchbuf,
                    REG_NEWLINE | mutt_which_case (searchbuf))) != 0) {
        regerror (err, &SearchRE, buffer, sizeof (buffer));
        mutt_error ("%s", buffer);
        regfree (&SearchRE);
        for (i = 0; i < maxLine; i++) {
          /* cleanup */
          if (lineInfo[i].search)
            FREE (&(lineInfo[i].search));
          lineInfo[i].search_cnt = -1;
        }
        SearchFlag = 0;
        SearchCompiled = 0;
      }
      else {
        SearchCompiled = 1;
        /* update the search pointers */
        i = 0;
        while (display_line (fp, &last_pos, &lineInfo, i, &lastLine,
                             &maxLine, M_SEARCH | (flags & M_PAGER_NSKIP),
                             &QuoteList, &q_level,
                             &force_redraw, &SearchRE) == 0) {
          i++;
          redraw |= REDRAW_SIDEBAR;
        }

        if (!SearchBack) {
          /* searching forward */
          for (i = topline; i < lastLine; i++) {
            if ((!hideQuoted || lineInfo[i].type != MT_COLOR_QUOTED) &&
                !lineInfo[i].continuation && lineInfo[i].search_cnt > 0)
              break;
          }

          if (i < lastLine)
            topline = i;
        }
        else {
          /* searching backward */
          for (i = topline; i >= 0; i--) {
            if ((!hideQuoted || lineInfo[i].type != MT_COLOR_QUOTED) &&
                !lineInfo[i].continuation && lineInfo[i].search_cnt > 0)
              break;
          }

          if (i >= 0)
            topline = i;
        }

        if (lineInfo[topline].search_cnt == 0) {
          SearchFlag = 0;
          mutt_error _("Not found.");
        }
        else
          SearchFlag = M_SEARCH;
      }
      redraw = REDRAW_BODY;
      break;

    case OP_SEARCH_TOGGLE:
      if (SearchCompiled) {
        SearchFlag ^= M_SEARCH;
        redraw = REDRAW_BODY;
      }
      break;

    case OP_HELP:
      /* don't let the user enter the help-menu from the help screen! */
      if (!InHelp) {
        InHelp = 1;
        mutt_help (MENU_PAGER);
        redraw = REDRAW_FULL;
        InHelp = 0;
      }
      else
        mutt_error _("Help is currently being shown.");
      break;

    case OP_PAGER_HIDE_QUOTED:
      if (has_types) {
        hideQuoted ^= M_HIDE;
        if (hideQuoted && lineInfo[topline].type == MT_COLOR_QUOTED)
          topline = upNLines (1, lineInfo, topline, hideQuoted);
        else
          redraw = REDRAW_BODY;
      }
      break;

    case OP_PAGER_SKIP_QUOTED:
      if (has_types) {
        int dretval = 0;
        int new_topline = topline;

        while ((new_topline < lastLine ||
                (0 == (dretval = display_line (fp, &last_pos, &lineInfo,
                                               new_topline, &lastLine,
                                               &maxLine, M_TYPES, &QuoteList,
                                               &q_level, &force_redraw,
                                               &SearchRE))))
               && lineInfo[new_topline].type != MT_COLOR_QUOTED) {
          redraw |= REDRAW_SIDEBAR;
          new_topline++;
        }

        if (dretval < 0) {
          mutt_error _("No more quoted text.");

          break;
        }

        while ((new_topline < lastLine ||
                (0 == (dretval = display_line (fp, &last_pos, &lineInfo,
                                               new_topline, &lastLine,
                                               &maxLine, M_TYPES, &QuoteList,
                                               &q_level, &force_redraw,
                                               &SearchRE))))
               && lineInfo[new_topline].type == MT_COLOR_QUOTED) {
          new_topline++;
          redraw |= REDRAW_SIDEBAR;
        }

        if (dretval < 0) {
          mutt_error _("No more unquoted text after quoted text.");

          break;
        }
        topline = new_topline;
      }
      break;

    case OP_PAGER_BOTTOM:      /* move to the end of the file */
      if (lineInfo[curline].offset < sb.st_size - 1) {
        i = curline;
        /* make sure the types are defined to the end of file */
        while (display_line (fp, &last_pos, &lineInfo, i, &lastLine,
                             &maxLine, has_types,
                             &QuoteList, &q_level, &force_redraw,
                             &SearchRE) == 0) {
          i++;
          redraw |= REDRAW_SIDEBAR;
        }
        topline = upNLines (bodylen, lineInfo, lastLine, hideQuoted);
      }
      else
        mutt_error _("Bottom of message is shown.");
      break;

    case OP_REDRAW:
      clearok (stdscr, TRUE);
      redraw = REDRAW_FULL;
      break;

    case OP_NULL:
      km_error_key (MENU_PAGER);
      break;

      /* --------------------------------------------------------------------
       * The following are operations on the current message rather than
       * adjusting the view of the message.
       */

    case OP_BOUNCE_MESSAGE:
      CHECK_MODE (IsHeader (extra) || IsMsgAttach (extra))
        CHECK_ATTACH;
      if (IsMsgAttach (extra))
        mutt_attach_bounce (extra->fp, extra->hdr,
                            extra->idx, extra->idxlen, extra->bdy);
      else
        ci_bounce_message (extra->hdr, &redraw);
      break;

    case OP_RESEND:
      CHECK_MODE (IsHeader (extra) || IsMsgAttach (extra))
        CHECK_ATTACH;
      if (IsMsgAttach (extra))
        mutt_attach_resend (extra->fp, extra->hdr,
                            extra->idx, extra->idxlen, extra->bdy);
      else
        mutt_resend_message (NULL, extra->ctx, extra->hdr);
      redraw = REDRAW_FULL;
      break;

    case OP_CHECK_TRADITIONAL:
      CHECK_MODE (IsHeader (extra));
      if (!(WithCrypto & APPLICATION_PGP))
        break;
      if (!(extra->hdr->security & PGP_TRADITIONAL_CHECKED)) {
        ch = -1;
        rc = OP_CHECK_TRADITIONAL;
      }
      break;

    case OP_CREATE_ALIAS:
      CHECK_MODE (IsHeader (extra) || IsMsgAttach (extra));
      if (IsMsgAttach (extra))
        mutt_create_alias (extra->bdy->hdr->env, NULL);
      else
        mutt_create_alias (extra->hdr->env, NULL);
      MAYBE_REDRAW (redraw);
      break;

    case OP_PURGE_MESSAGE:
    case OP_DELETE:
      CHECK_MODE (IsHeader (extra));
      CHECK_READONLY;

      CHECK_MX_ACL (Context, ACL_DELETE, _("Deletion"));

      mutt_set_flag (Context, extra->hdr, M_DELETE, 1);
      mutt_set_flag (Context, extra->hdr, M_PURGED,
                     ch != OP_PURGE_MESSAGE ? 0 : 1);
      if (option (OPTDELETEUNTAG))
        mutt_set_flag (Context, extra->hdr, M_TAG, 0);
      redraw = REDRAW_STATUS | REDRAW_INDEX;
      if (option (OPTRESOLVE)) {
        ch = -1;
        rc = OP_MAIN_NEXT_UNDELETED;
      }
      break;

    case OP_DELETE_THREAD:
    case OP_DELETE_SUBTHREAD:
      CHECK_MODE (IsHeader (extra));
      CHECK_READONLY;

      CHECK_MX_ACL (Context, ACL_DELETE, _("Deletion"));

      r = mutt_thread_set_flag (extra->hdr, M_DELETE, 1,
                                ch == OP_DELETE_THREAD ? 0 : 1);

      if (r != -1) {
        if (option (OPTDELETEUNTAG))
          mutt_thread_set_flag (extra->hdr, M_TAG, 0,
                                ch == OP_DELETE_THREAD ? 0 : 1);
        if (option (OPTRESOLVE)) {
          rc = OP_MAIN_NEXT_UNDELETED;
          ch = -1;
        }

        if (!option (OPTRESOLVE) && PagerIndexLines)
          redraw = REDRAW_FULL;
        else
          redraw = REDRAW_STATUS | REDRAW_INDEX;
      }
      break;

    case OP_DISPLAY_ADDRESS:
      CHECK_MODE (IsHeader (extra) || IsMsgAttach (extra));
      if (IsMsgAttach (extra))
        mutt_display_address (extra->bdy->hdr->env);
      else
        mutt_display_address (extra->hdr->env);
      break;

    case OP_ENTER_COMMAND:
      old_smart_wrap = option (OPTWRAP);
      old_markers = option (OPTMARKERS);
      old_PagerIndexLines = PagerIndexLines;

      CurrentMenu = MENU_PAGER;
      mutt_enter_command ();

      if (option (OPTNEEDRESORT)) {
        unset_option (OPTNEEDRESORT);
        CHECK_MODE (IsHeader (extra));
        set_option (OPTNEEDRESORT);
      }

      if (old_PagerIndexLines != PagerIndexLines) {
        if (index)
          mutt_menuDestroy (&index);
        index = NULL;
      }

      if (option (OPTWRAP) != old_smart_wrap ||
          option (OPTMARKERS) != old_markers) {
        if (flags & M_PAGER_RETWINCH) {
          ch = -1;
          rc = OP_REFORMAT_WINCH;
          continue;
        }

        /* count the real lines above */
        j = 0;
        for (i = 0; i <= topline; i++) {
          if (!lineInfo[i].continuation)
            j++;
        }

        /* we need to restart the whole thing */
        for (i = 0; i < maxLine; i++) {
          lineInfo[i].offset = 0;
          lineInfo[i].type = -1;
          lineInfo[i].continuation = 0;
          lineInfo[i].chunks = 0;
          lineInfo[i].search_cnt = -1;
          lineInfo[i].quote = NULL;

          safe_realloc (&(lineInfo[i].syntax), sizeof (struct syntax_t));
          if (SearchCompiled && lineInfo[i].search)
            FREE (&(lineInfo[i].search));
        }

        if (SearchCompiled) {
          regfree (&SearchRE);
          SearchCompiled = 0;
        }
        SearchFlag = 0;

        /* try to keep the old position */
        topline = 0;
        lastLine = 0;
        while (j > 0 && display_line (fp, &last_pos, &lineInfo, topline,
                                      &lastLine, &maxLine,
                                      (has_types ? M_TYPES : 0),
                                      &QuoteList, &q_level, &force_redraw,
                                      &SearchRE) == 0) {
          redraw |= REDRAW_SIDEBAR;
          if (!lineInfo[topline].continuation)
            j--;
          if (j > 0)
            topline++;
        }

        ch = 0;
      }

      if (option (OPTFORCEREDRAWPAGER))
        redraw = REDRAW_FULL;
      unset_option (OPTFORCEREDRAWINDEX);
      unset_option (OPTFORCEREDRAWPAGER);
      break;

    case OP_FLAG_MESSAGE:
      CHECK_MODE (IsHeader (extra));
      CHECK_READONLY;

      CHECK_MX_ACL (Context, ACL_WRITE, _("Flagging"));

      mutt_set_flag (Context, extra->hdr, M_FLAG, !extra->hdr->flagged);
      redraw = REDRAW_STATUS | REDRAW_INDEX;
      if (option (OPTRESOLVE)) {
        ch = -1;
        rc = OP_MAIN_NEXT_UNDELETED;
      }
      break;

    case OP_PIPE:
      CHECK_MODE (IsHeader (extra) || IsAttach (extra));
      if (IsAttach (extra))
        mutt_pipe_attachment_list (extra->fp, 0, extra->bdy, 0);
      else
        mutt_pipe_message (extra->hdr);
      MAYBE_REDRAW (redraw);
      break;

    case OP_PRINT:
      CHECK_MODE (IsHeader (extra) || IsAttach (extra));
      if (IsAttach (extra))
        mutt_print_attachment_list (extra->fp, 0, extra->bdy);
      else
        mutt_print_message (extra->hdr);
      break;

    case OP_MAIL:
      CHECK_MODE (IsHeader (extra) && !IsAttach (extra));
      CHECK_ATTACH;
      ci_send_message (0, NULL, NULL, extra->ctx, NULL);
      redraw = REDRAW_FULL;
      break;

#ifdef USE_NNTP
    case OP_POST:
      CHECK_MODE (IsHeader (extra) && !IsAttach (extra));
      CHECK_ATTACH;
      if (extra->ctx && extra->ctx->magic == M_NNTP &&
          !((NNTP_DATA *) extra->ctx->data)->allowed &&
          query_quadoption (OPT_TOMODERATED,
                            _
                            ("Posting to this group not allowed, may be moderated. Continue?"))
          != M_YES)
        break;
      ci_send_message (SENDNEWS, NULL, NULL, extra->ctx, NULL);
      redraw = REDRAW_FULL;
      break;

    case OP_FORWARD_TO_GROUP:
      CHECK_MODE (IsHeader (extra) || IsMsgAttach (extra));
      CHECK_ATTACH;
      if (extra->ctx && extra->ctx->magic == M_NNTP &&
          !((NNTP_DATA *) extra->ctx->data)->allowed &&
          query_quadoption (OPT_TOMODERATED,
                            _
                            ("Posting to this group not allowed, may be moderated. Continue?"))
          != M_YES)
        break;
      if (IsMsgAttach (extra))
        mutt_attach_forward (extra->fp, extra->hdr, extra->idx,
                             extra->idxlen, extra->bdy, SENDNEWS);
      else
        ci_send_message (SENDNEWS | SENDFORWARD, NULL, NULL, extra->ctx,
                         extra->hdr);
      redraw = REDRAW_FULL;
      break;

    case OP_FOLLOWUP:
      CHECK_MODE (IsHeader (extra) || IsMsgAttach (extra));
      CHECK_ATTACH;

      if (IsMsgAttach (extra))
        followup_to = extra->bdy->hdr->env->followup_to;
      else
        followup_to = extra->hdr->env->followup_to;

      if (!followup_to || safe_strcasecmp (followup_to, "poster") ||
          query_quadoption (OPT_FOLLOWUPTOPOSTER,
                            _("Reply by mail as poster prefers?")) != M_YES) {
        if (extra->ctx && extra->ctx->magic == M_NNTP
            && !((NNTP_DATA *) extra->ctx->data)->allowed
            && query_quadoption (OPT_TOMODERATED,
                                 _
                                 ("Posting to this group not allowed, may be moderated. Continue?"))
            != M_YES)
          break;
        if (IsMsgAttach (extra))
          mutt_attach_reply (extra->fp, extra->hdr, extra->idx,
                             extra->idxlen, extra->bdy, SENDNEWS | SENDREPLY);
        else
          ci_send_message (SENDNEWS | SENDREPLY, NULL, NULL,
                           extra->ctx, extra->hdr);
        redraw = REDRAW_FULL;
        break;
      }
#endif

    case OP_REPLY:
      CHECK_MODE (IsHeader (extra) || IsMsgAttach (extra));
      CHECK_ATTACH;
      if (IsMsgAttach (extra))
        mutt_attach_reply (extra->fp, extra->hdr, extra->idx,
                           extra->idxlen, extra->bdy, SENDREPLY);
      else
        ci_send_message (SENDREPLY, NULL, NULL, extra->ctx, extra->hdr);
      redraw = REDRAW_FULL;
      break;

    case OP_RECALL_MESSAGE:
      CHECK_MODE (IsHeader (extra) && !IsAttach (extra));
      CHECK_ATTACH;
      ci_send_message (SENDPOSTPONED, NULL, NULL, extra->ctx, extra->hdr);
      redraw = REDRAW_FULL;
      break;

    case OP_GROUP_REPLY:
      CHECK_MODE (IsHeader (extra) || IsMsgAttach (extra));
      CHECK_ATTACH;
      if (IsMsgAttach (extra))
        mutt_attach_reply (extra->fp, extra->hdr, extra->idx,
                           extra->idxlen, extra->bdy,
                           SENDREPLY | SENDGROUPREPLY);
      else
        ci_send_message (SENDREPLY | SENDGROUPREPLY, NULL, NULL, extra->ctx,
                         extra->hdr);
      redraw = REDRAW_FULL;
      break;

    case OP_LIST_REPLY:
      CHECK_MODE (IsHeader (extra) || IsMsgAttach (extra));
      CHECK_ATTACH;
      if (IsMsgAttach (extra))
        mutt_attach_reply (extra->fp, extra->hdr, extra->idx,
                           extra->idxlen, extra->bdy,
                           SENDREPLY | SENDLISTREPLY);
      else
        ci_send_message (SENDREPLY | SENDLISTREPLY, NULL, NULL, extra->ctx,
                         extra->hdr);
      redraw = REDRAW_FULL;
      break;

    case OP_FORWARD_MESSAGE:
      CHECK_MODE (IsHeader (extra) || IsMsgAttach (extra));
      CHECK_ATTACH;
      if (IsMsgAttach (extra))
        mutt_attach_forward (extra->fp, extra->hdr, extra->idx,
                             extra->idxlen, extra->bdy, 0);
      else
        ci_send_message (SENDFORWARD, NULL, NULL, extra->ctx, extra->hdr);
      redraw = REDRAW_FULL;
      break;

    case OP_DECRYPT_SAVE:
      if (!WithCrypto) {
        ch = -1;
        break;
      }
      /* fall through */
    case OP_SAVE:
      if (IsAttach (extra)) {
        mutt_save_attachment_list (extra->fp, 0, extra->bdy, extra->hdr,
                                   NULL);
        break;
      }
      /* fall through */
    case OP_COPY_MESSAGE:
    case OP_DECODE_SAVE:
    case OP_DECODE_COPY:
    case OP_DECRYPT_COPY:
      if (!WithCrypto && ch == OP_DECRYPT_COPY) {
        ch = -1;
        break;
      }
      CHECK_MODE (IsHeader (extra));
      if (mutt_save_message (extra->hdr,
                             (ch == OP_DECRYPT_SAVE) ||
                             (ch == OP_SAVE) || (ch == OP_DECODE_SAVE),
                             (ch == OP_DECODE_SAVE) || (ch == OP_DECODE_COPY),
                             (ch == OP_DECRYPT_SAVE)
                             || (ch == OP_DECRYPT_COPY) || 0, &redraw) == 0
          && (ch == OP_SAVE || ch == OP_DECODE_SAVE
              || ch == OP_DECRYPT_SAVE)) {
        if (option (OPTRESOLVE)) {
          ch = -1;
          rc = OP_MAIN_NEXT_UNDELETED;
        }
        else
          redraw |= REDRAW_STATUS | REDRAW_INDEX;
      }
      MAYBE_REDRAW (redraw);
      break;

    case OP_SHELL_ESCAPE:
      mutt_shell_escape ();
      MAYBE_REDRAW (redraw);
      break;

    case OP_TAG:
      CHECK_MODE (IsHeader (extra));
      mutt_set_flag (Context, extra->hdr, M_TAG, !extra->hdr->tagged);

      Context->last_tag = extra->hdr->tagged ? extra->hdr :
        ((Context->last_tag == extra->hdr && !extra->hdr->tagged)
         ? NULL : Context->last_tag);

      redraw = REDRAW_STATUS | REDRAW_INDEX;
      if (option (OPTRESOLVE)) {
        ch = -1;
        rc = OP_NEXT_ENTRY;
      }
      break;

    case OP_TOGGLE_NEW:
      CHECK_MODE (IsHeader (extra));
      CHECK_READONLY;

      CHECK_MX_ACL (Context, ACL_SEEN, _("Toggling"));

      if (extra->hdr->read || extra->hdr->old)
        mutt_set_flag (Context, extra->hdr, M_NEW, 1);
      else if (!first)
        mutt_set_flag (Context, extra->hdr, M_READ, 1);
      first = 0;
      Context->msgnotreadyet = -1;
      redraw = REDRAW_STATUS | REDRAW_INDEX;
      if (option (OPTRESOLVE)) {
        ch = -1;
        rc = OP_MAIN_NEXT_UNDELETED;
      }
      break;

    case OP_UNDELETE:
      CHECK_MODE (IsHeader (extra));
      CHECK_READONLY;

      CHECK_MX_ACL (Context, ACL_DELETE, _("Undeletion"));

      mutt_set_flag (Context, extra->hdr, M_DELETE, 0);
      mutt_set_flag (Context, extra->hdr, M_PURGED, 0);
      redraw = REDRAW_STATUS | REDRAW_INDEX;
      if (option (OPTRESOLVE)) {
        ch = -1;
        rc = OP_NEXT_ENTRY;
      }
      break;

    case OP_UNDELETE_THREAD:
    case OP_UNDELETE_SUBTHREAD:
      CHECK_MODE (IsHeader (extra));
      CHECK_READONLY;

      CHECK_MX_ACL (Context, ACL_DELETE, _("Undeletion"));

      r = mutt_thread_set_flag (extra->hdr, M_DELETE, 0,
                                ch == OP_UNDELETE_THREAD ? 0 : 1)
        + mutt_thread_set_flag (extra->hdr, M_PURGED, 0,
                                ch == OP_UNDELETE_THREAD ? 0 : 1);

      if (r > -1) {
        if (option (OPTRESOLVE)) {
          rc = (ch == OP_DELETE_THREAD) ?
            OP_MAIN_NEXT_THREAD : OP_MAIN_NEXT_SUBTHREAD;
          ch = -1;
        }

        if (!option (OPTRESOLVE) && PagerIndexLines)
          redraw = REDRAW_FULL;
        else
          redraw = REDRAW_STATUS | REDRAW_INDEX;
      }
      break;

    case OP_VERSION:
      mutt_version ();
      break;

    case OP_BUFFY_LIST:
      buffy_list ();
      redraw |= REDRAW_SIDEBAR;
      break;

    case OP_VIEW_ATTACHMENTS:
      if (flags & M_PAGER_ATTACHMENT) {
        ch = -1;
        rc = OP_ATTACH_COLLAPSE;
        break;
      }
      CHECK_MODE (IsHeader (extra));
      mutt_view_attachments (extra->hdr);
      if (extra->hdr->attach_del)
        Context->changed = 1;
      redraw = REDRAW_FULL;
      break;


    case OP_MAIL_KEY:
      if (!(WithCrypto & APPLICATION_PGP)) {
        ch = -1;
        break;
      }
      CHECK_MODE (IsHeader (extra));
      CHECK_ATTACH;
      ci_send_message (SENDKEY, NULL, NULL, extra->ctx, extra->hdr);
      redraw = REDRAW_FULL;
      break;


    case OP_FORGET_PASSPHRASE:
      crypt_forget_passphrase ();
      break;

    case OP_EXTRACT_KEYS:
      if (!WithCrypto) {
        ch = -1;
        break;
      }
      CHECK_MODE (IsHeader (extra));
      crypt_extract_keys_from_messages (extra->hdr);
      redraw = REDRAW_FULL;
      break;

    case OP_SIDEBAR_SCROLL_UP:
    case OP_SIDEBAR_SCROLL_DOWN:
    case OP_SIDEBAR_NEXT:
    case OP_SIDEBAR_NEXT_NEW:
    case OP_SIDEBAR_PREV:
    case OP_SIDEBAR_PREV_NEW:
      sidebar_scroll (ch, MENU_PAGER);
      break;
    default:
      ch = -1;
      break;
    }
  }

  fclose (fp);
  if (IsHeader (extra))
    Context->msgnotreadyet = -1;

  cleanup_quote (&QuoteList);

  for (i = 0; i < maxLine; i++) {
    FREE (&(lineInfo[i].syntax));
    if (SearchCompiled && lineInfo[i].search)
      FREE (&(lineInfo[i].search));
  }
  if (SearchCompiled) {
    regfree (&SearchRE);
    SearchCompiled = 0;
  }
  FREE (&lineInfo);
  if (index)
    mutt_menuDestroy (&index);
  return (rc != -1 ? rc : 0);
}
