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
static int prev_show_value;
static short saveSidebarWidth;
static char *entry = 0;

/* computes how many digets a number has;
 * FIXME move out to library?
 */
static int quick_log10 (int n) {
  int len = 0;

  for (; n > 9; len++, n /= 10);
  return (++len);
}

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
  TopBuffy = CurBuffy - (CurBuffy % lines);
  if (TopBuffy < 0)
    TopBuffy = 0;
}

/* compresses hierarchy in folder names;
 * FIXME move out to library?
 */
static char *shortened_hierarchy (char *box)
{
  int dots = 0;
  char *last_dot;
  int i, j;
  char *new_box;

  for (i = 0; i < mutt_strlen (box); ++i) {
    if (box[i] == '.')
      ++dots;
    else if (isupper (box[i]))
      return (safe_strdup (box));
  }
  last_dot = strrchr (box, '.');
  if (last_dot) {
    ++last_dot;
    new_box = safe_malloc (mutt_strlen (last_dot) + 2 * dots + 1);
    new_box[0] = box[0];
    for (i = 1, j = 1; i < mutt_strlen (box); ++i) {
      if (box[i] == '.') {
        new_box[j++] = '.';
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

/* print single item
 * FIXME this is completely fucked up right now
 */
char *make_sidebar_entry (char *box, int size, int new, int flagged)
{
  int i = 0, dlen, max, shortened = 0;
  int offset;

  if (SidebarWidth > COLS)
    SidebarWidth = COLS;

  dlen = mutt_strlen (SidebarDelim);
  max = SidebarWidth - dlen - 1;

  safe_realloc (&entry, SidebarWidth + 1);
  entry[SidebarWidth] = 0;
  for (; i < SidebarWidth; entry[i++] = ' ');
#if USE_IMAP
  if (ImapHomeNamespace && mutt_strlen (ImapHomeNamespace) > 0) {
    if (strncmp (box, ImapHomeNamespace, mutt_strlen (ImapHomeNamespace)) == 0
        && strcmp (box, ImapHomeNamespace) != 0) {
      box += mutt_strlen (ImapHomeNamespace) + 1;
    }
  }
#endif
  max -= quick_log10 (size);
  if (new)
    max -= quick_log10 (new) + 2;
  if (flagged > 0)
    max -= quick_log10 (flagged) + 2;
  if (option (OPTSHORTENHIERARCHY) && mutt_strlen (box) > max) {
    box = shortened_hierarchy (box);
    shortened = 1;
  }
  i = mutt_strlen (box);
  strncpy (entry, box, i < SidebarWidth - dlen ? i : SidebarWidth - dlen);

  if (new) {
    if (flagged > 0) {
      offset =
        SidebarWidth - 5 - quick_log10 (size) - dlen - quick_log10 (new) -
        quick_log10 (flagged);
      if (offset < 0)
        offset = 0;
      snprintf (entry + offset, SidebarWidth - dlen - offset + 1,
                "% d(%d)[%d]", size, new, flagged);
    }
    else {
      offset =
        SidebarWidth - 3 - quick_log10 (size) - dlen - quick_log10 (new);
      if (offset < 0)
        offset = 0;
      snprintf (entry + offset, SidebarWidth - dlen - offset + 1,
                "% d(%d)", size, new);
    }
  }
  else {
    if (flagged > 0) {
      offset =
        SidebarWidth - 3 - quick_log10 (size) - dlen - quick_log10 (flagged);
      if (offset < 0)
        offset = 0;
      snprintf (entry + offset, SidebarWidth - dlen - offset + 1,
                "% d[%d]", size, flagged);
    }
    else {
      offset = SidebarWidth - 1 - quick_log10 (size) - dlen;
      if (offset < 0)
        offset = 0;
      snprintf (entry + offset, SidebarWidth - dlen - offset + 1,
                "% d", size);
    }
  }

  if (option (OPTSHORTENHIERARCHY) && shortened) {
    free (box);
  }
  return entry;
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
 * FIXME this is completely fucked up right now
 */
int sidebar_draw (int menu)
{

  int lines = option (OPTHELP) ? 1 : 0;
  BUFFY *tmp;
  int i = 0;
  short delim_len = mutt_strlen (SidebarDelim);

  /* initialize first time */
  if (!initialized) {
    prev_show_value = option (OPTMBOXPANE);
    saveSidebarWidth = SidebarWidth;
    if (!option (OPTMBOXPANE))
      SidebarWidth = 0;
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
      && mutt_strlen (SidebarDelim) >= SidebarWidth) {
    mutt_error (_("Value for sidebar_delim is too long. Disabling sidebar."));
    sleep (2);
    unset_option (OPTMBOXPANE);
    return (0);
  }

  if (SidebarWidth == 0 || !option (OPTMBOXPANE))
    return 0;

  /* draw the divider */
  SETCOLOR (MT_COLOR_SIDEBAR);
  for (lines = 1;
       lines < LINES - 1 - (menu != MENU_PAGER || option (OPTSTATUSONTOP));
       lines++) {
    move (lines, SidebarWidth - delim_len);
    if (option (OPTASCIICHARS))
      addstr (NONULL (SidebarDelim));
    else if (!option (OPTASCIICHARS) && !mutt_strcmp (SidebarDelim, "|"))
      addch (ACS_VLINE);
    else if ((Charset_is_utf8) && !mutt_strcmp (SidebarDelim, "|"))
      addstr ("\342\224\202");
    else
      addstr (NONULL (SidebarDelim));
  }
  SETCOLOR (MT_COLOR_NORMAL);

  if (list_empty(Incoming))
    return 0;

  lines = option (OPTHELP) ? 1 : 0;     /* go back to the top */

  calc_boundaries (menu);

  for (i = TopBuffy; i < Incoming->length && lines < LINES - 1 - 
       (menu != MENU_PAGER || option (OPTSTATUSONTOP)); i++) {
    tmp = (BUFFY*) Incoming->data[i];
    if (i == CurBuffy)
      SETCOLOR (MT_COLOR_INDICATOR);
    else if (tmp->msg_flagged > 0)
      SETCOLOR (MT_COLOR_FLAGGED);
    else if (tmp->msg_unread > 0)
      SETCOLOR (MT_COLOR_NEW);
    else
      SETCOLOR (MT_COLOR_NORMAL);

    move (lines, 0);
    if (option (OPTSIDEBARNEWMAILONLY)) {
      if (tmp->msg_unread > 0) {
        if (Context && !mutt_strcmp (tmp->path, Context->path)) {
          printw ("%.*s", SidebarWidth - delim_len,
                  make_sidebar_entry (basename (tmp->path),
                                      Context->msgcount, Context->unread,
                                      Context->flagged));
          tmp->msg_unread = Context->unread;
          tmp->msgcount = Context->msgcount;
          tmp->msg_flagged = Context->flagged;
        }
        else
          printw ("%.*s", SidebarWidth - delim_len,
                  make_sidebar_entry (basename (tmp->path),
                                      tmp->msgcount, tmp->msg_unread,
                                      tmp->msg_flagged));
        lines++;
      }
    }
    else {
      if (Context && !strcmp (tmp->path, Context->path)) {
        printw ("%.*s", SidebarWidth - delim_len,
                make_sidebar_entry (basename (tmp->path),
                                    Context->msgcount, Context->unread,
                                    Context->flagged));
        tmp->msg_unread = Context->unread;
        tmp->msgcount = Context->msgcount;
        tmp->msg_flagged = Context->flagged;
      }
      else
        printw ("%.*s", SidebarWidth - delim_len,
                make_sidebar_entry (basename (tmp->path),
                                    tmp->msgcount, tmp->msg_unread,
                                    tmp->msg_flagged));
      lines++;
    }
  }
  SETCOLOR (MT_COLOR_NORMAL);
  for (; lines < LINES - 1 - (menu != MENU_PAGER || option (OPTSTATUSONTOP));
       lines++) {
    int i = 0;

    move (lines, 0);
    for (; i < SidebarWidth - delim_len; i++)
      addch (' ');
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
    if (((BUFFY*) Incoming->data[i++])->msg_unread)
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
    if (((BUFFY*) Incoming->data[i--])->msg_unread)
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
