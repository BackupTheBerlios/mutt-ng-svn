/*
 * Copyright notice from original mutt:
 * Copyright (C) ????-2004 Justin Hibbits <jrh29@po.cwru.edu>
 * Copyright (C) 2004 Thomer M. Gil <mutt@thomer.com>
 *
 * Parts were written/modified by:
 * Rocco Rutte <pdmef@cs.tu-berlin.de>
 * Nico Golde <nico@ngolde.de>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#include "mutt.h"
#include "mutt_menu.h"
#include "mutt_curses.h"
#include "sidebar.h"
#include "buffy.h"
#include "keymap.h"

#include "lib/mem.h"
#include "lib/str.h"
#include "lib/intl.h"

#include <libgen.h>
#include <ctype.h>

static int TopBuffy = 0;
static int CurBuffy = 0;
static int known_lines = 0;
static short initialized = 0;
static short prev_show_value;

/* computes first entry to be shown */
static void calc_boundaries (void) {
  if (list_empty(Incoming))
    return;
  if (CurBuffy < 0 || CurBuffy >= Incoming->length)
    CurBuffy = 0;
  if (TopBuffy < 0 || TopBuffy >= Incoming->length)
    TopBuffy = 0;

  if (option (OPTSIDEBARNEWMAILONLY)) {
    int i = CurBuffy;
    TopBuffy = CurBuffy - 1;
    while (i >= 0) {
      if (((BUFFY*) Incoming->data[i])->new > 0)
        TopBuffy = i;
      i--;
    }
  } else if (known_lines>0)
    TopBuffy = CurBuffy - (CurBuffy % known_lines);
  if (TopBuffy < 0)
    TopBuffy = 0;
}

static char *shortened_hierarchy (char *box, int maxlen)
{
  int dots = 0;
  char *last_dot = NULL;
  int i, j, len = str_len (box);
  char *new_box;

  if (!SidebarBoundary || !*SidebarBoundary)
    return (str_dup (box));

  for (i = 0; i < len; ++i) {
    if (strchr (SidebarBoundary, box[i])) {
      ++dots;
      last_dot = &box[i];
    }
  }

  if (last_dot) {
    ++last_dot;
    new_box = mem_malloc (maxlen + 1);
    new_box[0] = box[0];
    for (i = 1, j = 1; j < maxlen && i < len; ++i) {
      if (strchr (SidebarBoundary, box[i])) {
        new_box[j++] = box[i];
        new_box[j] = 0;
        if (&box[i + 1] != last_dot || j + str_len (last_dot) > maxlen) {
          new_box[j++] = box[i + 1];
          new_box[j] = 0;
        } else {
          strcat (&new_box[j], last_dot);
          break;
        }
      }
    }
    return new_box;
  }
  return str_dup (box);
}

static const char* sidebar_number_format (char* dest, size_t destlen, char op,
                                          const char* src, const char* fmt,
                                          const char* ifstr, const char* elstr,
                                          unsigned long data, format_flag flags) {
  char tmp[SHORT_STRING];
  BUFFY* b = (BUFFY*) Incoming->data[data];
  int opt = flags & M_FORMAT_OPTIONAL;
  int c = Context && str_eq (Context->path, b->path);

  switch (op) {
    /* deleted */
    case 'd':
      if (!opt) {
        snprintf (tmp, sizeof (tmp), "%%%sd", fmt);
        snprintf (dest, destlen, tmp, c ? Context->deleted : 0);
      } else if ((c && Context->deleted == 0) || !c)
        opt = 0;
      break;
    /* flagged */
    case 'F':
    case 'f':                   /* for compatibility */
      if (!opt) {
        snprintf (tmp, sizeof (tmp), "%%%sd", fmt);
        snprintf (dest, destlen, tmp, c ? Context->flagged : b->msg_flagged);
      } else if ((c && Context->flagged == 0) || (!c && b->msg_flagged == 0))
        opt = 0;
      break;
    /* total */
    case 'c':                   /* for compatibility */
    case 'm':
      if (!opt) {
        snprintf (tmp, sizeof (tmp), "%%%sd", fmt);
        snprintf (dest, destlen, tmp, c ? Context->msgcount : b->msgcount);
      } else if ((c && Context->msgcount == 0) || (!c && b->msgcount == 0))
        opt = 0;
      break;
    /* total shown, i.e. not hidden by limit */
    case 'M':
      if (!opt) {
        snprintf (tmp, sizeof (tmp), "%%%sd", fmt);
        snprintf (dest, destlen, tmp, c ? Context->vcount : 0);
      } else if ((c && Context->vcount == 0) || !c)
        opt = 0;
      break;
    /* new */
    case 'n':
      if (!opt) {
        snprintf (tmp, sizeof (tmp), "%%%sd", fmt);
        snprintf (dest, destlen, tmp, c ? Context->new : b->new);
      } else if ((c && Context->new == 0) || (!c && b->new == 0))
        opt = 0;
      break;
    /* unread */
    case 'u':
      if (!opt) {
        snprintf (tmp, sizeof (tmp), "%%%sd", fmt);
        snprintf (dest, destlen, tmp, c ? Context->unread : b->msg_unread);
      } else if ((c && Context->unread == 0) || (!c && b->msg_unread == 0))
        opt = 0;
      break;
    /* tagged */
    case 't':
      if (!opt) {
        snprintf (tmp, sizeof (tmp), "%%%sd", fmt);
        snprintf (dest, destlen, tmp, c ? Context->tagged : 0);
      } else if ((c && Context->tagged == 0) || !c)
        opt = 0;
      break;
  }

  if (opt)
    mutt_FormatString (dest, destlen, ifstr, sidebar_number_format,
                       data, M_FORMAT_OPTIONAL);
  else if (flags & M_FORMAT_OPTIONAL)
    mutt_FormatString (dest, destlen, elstr, sidebar_number_format,
                       data, M_FORMAT_OPTIONAL);
  return (src);
}

int sidebar_need_count (void) {
  if (!option (OPTMBOXPANE) || SidebarWidth == 0 ||
      !SidebarNumberFormat || !*SidebarNumberFormat)
    return (0);
  return (1);
}

/* print single item
 * returns:
 *      0       item was not printed ('cause of $sidebar_newmail_only)
 *      1       item was printed
 */
int make_sidebar_entry (char* box, int idx, size_t len)
{
  int shortened = 0, lencnt = 0;
  char no[SHORT_STRING], entry[SHORT_STRING];
#if USE_IMAP
  int l = str_len (ImapHomeNamespace);
#endif
  int l_m = str_len (Maildir);

  if (SidebarWidth > COLS)
    SidebarWidth = COLS;

  if (option (OPTSIDEBARNEWMAILONLY) && box && Context && Context->path && 
      !str_eq (Context->path, box) && ((BUFFY*) Incoming->data[idx])->new == 0)
    /* if $sidebar_newmail_only is set, don't display the
     * box only if it's not the currently opened
     * (i.e. always display the currently opened) */
    return (0);

  mutt_FormatString (no, len, NONULL (SidebarNumberFormat),
                     sidebar_number_format, idx, M_FORMAT_OPTIONAL);
  lencnt = str_len (no);
  memset (&entry, ' ', sizeof (entry));

#if USE_IMAP
  if (l > 0 && str_ncmp (box, ImapHomeNamespace, l) == 0 && 
      str_len (box) > l)
    box += l + 1; /* we're trimming the ImapHomeNamespace, the "+ 1" is for the separator */
  else
#endif
  if (l_m > 0 && str_ncmp (box, Maildir, l_m) == 0 && 
      str_len (box) > l_m) {
    box += l_m;
    if (Maildir[strlen(Maildir)-1]!='/') {
      box += 1;
    }
  } else
    box = basename (box);

  if (option (OPTSHORTENHIERARCHY) && str_len (box) > len-lencnt-1) {
    box = shortened_hierarchy (box, len-lencnt-1);
    shortened = 1;
  }

  snprintf (entry, len-lencnt, "%s", box);
  entry[str_len (entry)] = ' ';
  strncpy (entry + (len - lencnt), no, lencnt);

  addnstr (entry, len);

  if (shortened)
    mem_free(&box);

  return (1);
}

/* returns folder name of currently 
 * selected folder for <sidebar-open>
 */
const char* sidebar_get_current (void) {
  if (list_empty(Incoming))
    return (NULL);
  return ((char*) ((BUFFY*) Incoming->data[CurBuffy])->path);
}

/* internally sets item to buf */
void sidebar_set_current (const char* buf) {
  int i = buffy_lookup (buf);
  if (i >= 0) {
    CurBuffy = i;
    calc_boundaries();
  }
}

/* fix counters for a context
 * FIXME since ctx must not be of our business, move it elsewhere
 */
void sidebar_set_buffystats (CONTEXT* Context) {
  int i = 0;
  BUFFY* tmp = NULL;
  if (!Context || list_empty(Incoming) || (i = buffy_lookup (Context->path)) < 0)
    return;
  tmp = (BUFFY*) Incoming->data[i];
  tmp->new = Context->new;
  tmp->msg_unread = Context->unread;
  tmp->msgcount = Context->msgcount;
  tmp->msg_flagged = Context->flagged;
}

void sidebar_draw_frames (void) {
  size_t i,delim_len;

  if (!option(OPTMBOXPANE) || SidebarWidth==0) 
    return;

  delim_len=str_len(NONULL(SidebarDelim));

  /* draw vertical delimiter */
  SETCOLOR (MT_COLOR_SIDEBAR);
  for (i = 0; i < LINES-1; i++) {
    move (i, SidebarWidth - delim_len);
    if (option (OPTASCIICHARS))
      addstr (NONULL (SidebarDelim));
    else if (!option (OPTASCIICHARS) && !str_cmp (SidebarDelim, "|"))
      addch (ACS_VLINE);
    else if ((Charset_is_utf8) && !str_cmp (SidebarDelim, "|"))
      addstr ("\342\224\202");
    else
      addstr (NONULL (SidebarDelim));
  }

  /* fill "gaps" at top+bottom */
  SETCOLOR(MT_COLOR_STATUS);
  for (i=0; i<SidebarWidth; i++) {
    /*
     * if we don't have $status_on_top and have $help, fill top
     * gap with spaces to get bg color
     */
    if (option(OPTSTATUSONTOP) || option(OPTHELP)) {
      move(0,i);
      addch(' ');
    }
    /*
      * if we don't have $status_on_top or we have $help, fill bottom
      * gap with spaces to get bg color
      */
    if (!option(OPTSTATUSONTOP) || option(OPTHELP)) {
      move(LINES-2,i);
      addch(' ');
    }
  }
  SETCOLOR (MT_COLOR_NORMAL);
}

/* actually draws something
 * FIXME this needs some clue when to do it
 */
int sidebar_draw (int menu) {
  int first_line = option (OPTSTATUSONTOP) ? 1 : option (OPTHELP) ? 1 : 0,
      last_line = LINES - 2 + (option (OPTSTATUSONTOP) && !option (OPTHELP) ? 1 : 0),
      i = 0,line;
  BUFFY *tmp;
  size_t delim_len = str_len (SidebarDelim);
  char blank[SHORT_STRING];

  known_lines=last_line-first_line;

  /* initialize first time */
  if (!initialized) {
    prev_show_value = option (OPTMBOXPANE);
    initialized = 1;
  }

  if (TopBuffy==0 || CurBuffy==0)
    calc_boundaries();

  /* save or restore the value SidebarWidth */
  if (prev_show_value != option (OPTMBOXPANE)) {
    if (!prev_show_value && option (OPTMBOXPANE)) {
      /* after toggle: force recounting of all mail */
      buffy_check (2);
    }
    prev_show_value = option (OPTMBOXPANE);
  }

  if (SidebarWidth > 0 && option (OPTMBOXPANE)
      && str_len (SidebarDelim) >= SidebarWidth) {
    mutt_error (_("Value for sidebar_delim is too long. Disabling sidebar."));
    sleep (2);
    unset_option (OPTMBOXPANE);
    return (0);
  }

  if (SidebarWidth == 0 || !option (OPTMBOXPANE))
    return 0;

  sidebar_draw_frames();

  if (list_empty(Incoming))
    return 0;

  /* actually print items */
  for (i = TopBuffy, line=first_line; i < Incoming->length && line < last_line; i++) {
    tmp = (BUFFY*) Incoming->data[i];

    if (i == CurBuffy)
      SETCOLOR (MT_COLOR_INDICATOR);
    else if (tmp->new > 0)
      SETCOLOR (MT_COLOR_NEW);
    else if (tmp->msg_flagged > 0)
      SETCOLOR (MT_COLOR_FLAGGED);
    else
      SETCOLOR (MT_COLOR_NORMAL);

    move (line, 0);
    line += make_sidebar_entry (tmp->path, i, SidebarWidth-delim_len);
  }

  SETCOLOR (MT_COLOR_NORMAL);

  /* fill with blanks to bottom */
  memset (&blank, ' ', sizeof (blank));
  for (; line < last_line; line++) {
    move (line, 0);
    addnstr (blank, SidebarWidth-delim_len);
  }
  return 0;
}

/* returns index of new item with new mail or -1 */
static int exist_next_new () {
  int i = 0;
  if (list_empty(Incoming))
    return (-1);
  i = CurBuffy + 1;
  while (i < Incoming->length)
    if (((BUFFY*) Incoming->data[i++])->new > 0)
      return (i-1);
  return (-1);
}

/* returns index of prev item with new mail or -1 */
static int exist_prev_new () {
  int i = 0;
  if (list_empty(Incoming))
    return (-1);
  i = CurBuffy - 1;
  while (i >= 0)
    if (((BUFFY*) Incoming->data[i--])->new > 0)
      return (i+1);
  return (-1);
}

void sidebar_scroll (int op, int menu) {
  int i = 0;

  if (!SidebarWidth || list_empty(Incoming))
    return;

  switch (op) {
  case OP_SIDEBAR_NEXT:
    if (!option (OPTSIDEBARNEWMAILONLY)) {
      if (CurBuffy + 1 == Incoming->length) {
        mutt_error (_("You are on the last mailbox."));
        return;
      }
      CurBuffy++;
      break;
    }                           /* the fall-through is intentional */
  case OP_SIDEBAR_NEXT_NEW:
    if ((i = exist_next_new ()) < 0) {
      mutt_error (_("No next mailboxes with new mail."));
      return;
    }
    else
      CurBuffy = i;
    break;
  case OP_SIDEBAR_PREV:
    if (!option (OPTSIDEBARNEWMAILONLY)) {
      if (CurBuffy == 0) {
        mutt_error (_("You are on the first mailbox."));
        return;
      }
      CurBuffy--;
      break;
    }                           /* the fall-through is intentional */
  case OP_SIDEBAR_PREV_NEW:
    if ((i = exist_prev_new ()) < 0) {
      mutt_error (_("No previous mailbox with new mail."));
      return;
    }
    else
      CurBuffy = i;
    break;

  case OP_SIDEBAR_SCROLL_UP:
    if (CurBuffy == 0) {
      mutt_error (_("You are on the first mailbox."));
      return;
    }
    CurBuffy -= known_lines;
    if (CurBuffy < 0)
      CurBuffy = 0;
    break;
  case OP_SIDEBAR_SCROLL_DOWN:
    if (CurBuffy + 1 == Incoming->length) {
      mutt_error (_("You are on the last mailbox."));
      return;
    }
    CurBuffy += known_lines;
    if (CurBuffy >= Incoming->length)
      CurBuffy = Incoming->length - 1;
    break;
  default:
    return;
  }
  calc_boundaries ();
  sidebar_draw (menu);
}
