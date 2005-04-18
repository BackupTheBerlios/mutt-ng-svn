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
static short saveSidebarWidth;

/* computes first entry to be shown */
void calc_boundaries (int menu)
{
  int lines = 0;

  if (list_empty(Incoming))
    return;
  /* correct known_lines if it has changed because of a window resize */
  if (known_lines != LINES)
    known_lines = LINES;
  lines = LINES - 2 - (menu != MENU_PAGER || option (OPTSTATUSONTOP));
  if (option (OPTSIDEBARNEWMAILONLY)) {
    int i = CurBuffy;
    TopBuffy = CurBuffy - 1;
    while (i >= 0) {
      if (((BUFFY*) Incoming->data[i])->new > 0)
        TopBuffy = i;
      i--;
    }
  } else
    TopBuffy = CurBuffy - (CurBuffy % lines);
  if (TopBuffy < 0)
    TopBuffy = 0;
}

static char *shortened_hierarchy (char *box)
{
  int dots = 0;
  char *last_dot = NULL;
  int i, j;
  char *new_box;

  if (!SidebarBoundary || !*SidebarBoundary)
    return (safe_strdup (box));

  for (i = 0; i < safe_strlen (box); ++i) {
    if (strchr (SidebarBoundary, box[i]))
      ++dots;
  }
  for (i = safe_strlen (box)-1; i >= 0; i--)
    if (strchr (SidebarBoundary, box[i])) {
      last_dot = &box[i];
      break;
    }
  if (last_dot) {
    ++last_dot;
    new_box = safe_malloc (safe_strlen (last_dot) + 2 * dots + 1);
    new_box[0] = box[0];
    for (i = 1, j = 1; i < safe_strlen (box); ++i) {
      if (strchr (SidebarBoundary, box[i])) {
        new_box[j++] = box[i];
        new_box[j] = 0;
        if (&box[i + 1] != last_dot) {
          new_box[j++] = box[i + 1];
          new_box[j] = 0;
        }
        else {
          strcat (&new_box[j], last_dot);
          break;
        }
      }
    }
    return new_box;
  }
  return safe_strdup (box);
}

static const char* sidebar_number_format (char* dest, size_t destlen, char op,
                                          const char* src, const char* fmt,
                                          const char* ifstr, const char* elstr,
                                          unsigned long data, format_flag flags) {
  char tmp[SHORT_STRING];
  BUFFY* b = (BUFFY*) Incoming->data[data];
  int opt = flags & M_FORMAT_OPTIONAL;
  int c = Context && safe_strcmp (Context->path, b->path) == 0;

  switch (op) {
    case 'c':
      snprintf (tmp, sizeof (tmp), "%%%sd", fmt);
      snprintf (dest, destlen, tmp, c ? Context->msgcount : b->msgcount);
      break;
    case 'n':
      if (!opt) {
        snprintf (tmp, sizeof (tmp), "%%%sd", fmt);
        snprintf (dest, destlen, tmp, c ? Context->unread : b->msg_unread);
      } else if ((c && Context->unread == 0) || (!c && b->msg_unread == 0))
        opt = 0;
      break;
    case 'f':
      if (!opt) {
        snprintf (tmp, sizeof (tmp), "%%%sd", fmt);
        snprintf (dest, destlen, tmp, c ? Context->flagged : b->msg_flagged);
      } else if ((c && Context->flagged == 0) || (!c && b->msg_flagged == 0))
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
  int l = safe_strlen (ImapHomeNamespace);
#endif

  if (SidebarWidth > COLS)
    SidebarWidth = COLS;

  if (option (OPTSIDEBARNEWMAILONLY) && box && Context && Context->path && 
      safe_strcmp (Context->path, box) != 0 && 
      ((BUFFY*) Incoming->data[idx])->new == 0)
    /* if $sidebar_newmail_only is set, don't display the
     * box only if it's not the currently opened
     * (i.e. always display the currently opened) */
    return (0);

  mutt_FormatString (no, len, NONULL (SidebarNumberFormat),
                     sidebar_number_format, idx, M_FORMAT_OPTIONAL);
  lencnt = safe_strlen (no);
  memset (&entry, ' ', sizeof (entry));

#if USE_IMAP
  if (l > 0 && safe_strncmp (box, ImapHomeNamespace, l) == 0 && 
      safe_strlen (box) > l)
    box += l + 1;
  else
#endif
    box = basename (box);

  if (option (OPTSHORTENHIERARCHY) && safe_strlen (box) > len-lencnt-1) {
    box = shortened_hierarchy (box);
    shortened = 1;
  }

  snprintf (entry, len-lencnt-1, "%s", box);
  entry[safe_strlen (entry)] = ' ';
  strncpy (entry + (len - lencnt), no, lencnt);

  addnstr (entry, len);

  if (shortened)
    FREE(&box);

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
  if (i >= 0)
    CurBuffy = i;
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

/* actually draws something
 * FIXME this needs some clue when to do it
 */
int sidebar_draw (int menu)
{

  int lines = option (OPTHELP) ? 1 : 0, draw_devider = 1, i = 0;
  BUFFY *tmp;
  short delim_len = safe_strlen (SidebarDelim);
  char blank[SHORT_STRING];

  /* initialize first time */
  if (!initialized) {
    prev_show_value = option (OPTMBOXPANE);
    saveSidebarWidth = SidebarWidth;
    if (!option (OPTMBOXPANE)){
      SidebarWidth = 0;
      draw_devider = 1;
    }
    initialized = 1;
  }

  /* save or restore the value SidebarWidth */
  if (prev_show_value != option (OPTMBOXPANE)) {
    if (prev_show_value && !option (OPTMBOXPANE)) {
      saveSidebarWidth = SidebarWidth;
      SidebarWidth = 0;
    }
    else if (!prev_show_value && option (OPTMBOXPANE)) {
      SidebarWidth = saveSidebarWidth;
      /* after toggle: force recounting of all mail */
      mutt_buffy_check (2);
    }
    prev_show_value = option (OPTMBOXPANE);
  }

  if (SidebarWidth > 0 && option (OPTMBOXPANE)
      && safe_strlen (SidebarDelim) >= SidebarWidth) {
    mutt_error (_("Value for sidebar_delim is too long. Disabling sidebar."));
    sleep (2);
    unset_option (OPTMBOXPANE);
    return (0);
  }

  if (SidebarWidth == 0 || !option (OPTMBOXPANE))
    return 0;
  /* draw devider only if necessary (if the sidebar becomes visible e.g.)*/
  if (draw_devider == 1){
    /* draw the divider */
    SETCOLOR (MT_COLOR_SIDEBAR);
    for (lines = 1;
         lines < LINES - 1 - (menu != MENU_PAGER || option (OPTSTATUSONTOP));
         lines++) {
      move (lines, SidebarWidth - delim_len);
      if (option (OPTASCIICHARS))
        addstr (NONULL (SidebarDelim));
      else if (!option (OPTASCIICHARS) && !safe_strcmp (SidebarDelim, "|"))
        addch (ACS_VLINE);
      else if ((Charset_is_utf8) && !safe_strcmp (SidebarDelim, "|"))
        addstr ("\342\224\202");
      else
        addstr (NONULL (SidebarDelim));
    }
  }
  SETCOLOR (MT_COLOR_NORMAL);

  if (list_empty(Incoming))
    return 0;
  lines = option (OPTHELP) ? 1 : 0;     /* go back to the top */
  calc_boundaries (menu);

  /* actually print items */
  for (i = TopBuffy; i < Incoming->length && lines < LINES - 1 - 
       (menu != MENU_PAGER || option (OPTSTATUSONTOP)); i++) {
    tmp = (BUFFY*) Incoming->data[i];

    if (i == CurBuffy)
      SETCOLOR (MT_COLOR_INDICATOR);
    else if (tmp->msg_flagged > 0)
      SETCOLOR (MT_COLOR_FLAGGED);
    else if (tmp->new > 0)
      SETCOLOR (MT_COLOR_NEW);
    else
      SETCOLOR (MT_COLOR_NORMAL);

    move (lines, 0);
    lines += make_sidebar_entry (tmp->path, i, SidebarWidth-delim_len);
  }

  /* fill with blanks to bottom */
  memset (&blank, ' ', sizeof (blank));
  SETCOLOR (MT_COLOR_NORMAL);
  for (; lines < LINES - 1 - (menu != MENU_PAGER || option (OPTSTATUSONTOP)); lines++) {
    move (lines, 0);
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
    if (TopBuffy == 0) {
      mutt_error (_("You are on the first mailbox."));
      return;
    }
    CurBuffy -= known_lines;
    if (CurBuffy < 0)
      CurBuffy = 0;
    break;
  case OP_SIDEBAR_SCROLL_DOWN:
    if (TopBuffy + known_lines >= Incoming->length) {
      mutt_error (_("You are on the last mailbox."));
      return;
    }
    CurBuffy += known_lines;
    if (CurBuffy >= Incoming->length)
      CurBuffy = Incoming->length;
    break;
  default:
    return;
  }
  calc_boundaries (menu);
  sidebar_draw (menu);
}
