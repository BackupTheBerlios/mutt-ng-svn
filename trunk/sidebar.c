/*
 * Copyright (C) ????-2004 Justin Hibbits <jrh29@po.cwru.edu>
 * Copyright (C) 2004 Thomer M. Gil <mutt@thomer.com>
 * 
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 * 
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 * 
 *     You should have received a copy of the GNU General Public License
 *     along with this program; if not, write to the Free Software
 *     Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.
 */ 



#include "mutt.h"
#include "mutt_menu.h"
#include "mutt_curses.h"
#include "sidebar.h"
#include "buffy.h"
#include <libgen.h>
#include "keymap.h"
#include <ctype.h>

/*BUFFY *CurBuffy = 0;*/
static BUFFY *TopBuffy = 0;
static BUFFY *BottomBuffy = 0;
static int known_lines = 0;
static short initialized = 0;
static int prev_show_value;
static short saveSidebarWidth;
static char *entry = 0;

static int quick_log10(int n)
{
  int len = 0;
  for (; n > 9; len++, n /= 10)
    ;
  return (++len);
}

/* CurBuffy should contain a valid buffy 
 * mailbox before calling this function!!! */
void calc_boundaries (int menu)
{
  BUFFY *tmp = Incoming;
  int position;
  int i,count, mailbox_position;

  /* correct known_lines if it has changed because of a window resize */
  if ( known_lines != LINES ) {
    known_lines = LINES;
  }
  /* fix all the prev links on all the mailboxes
   * FIXME move this over to buffy.c where it belongs */
  for ( ; tmp->next != 0; tmp = tmp->next )
    tmp->next->prev = tmp;

  /* calculate the position of the current mailbox */
  position = 1;
  tmp = Incoming;
  while (tmp != CurBuffy)
  {
    position++;
    tmp = tmp->next;
  }
  /* calculate the size of the screen we can use */
  count = LINES - 2 - (menu != MENU_PAGER || option (OPTSTATUSONTOP));
  /* calculate the position of the current mailbox on the screen */
  mailbox_position = position%count;
  if (mailbox_position == 0) mailbox_position=count;
  /* determine topbuffy */
  TopBuffy = CurBuffy;
  for(i = mailbox_position; i >1; i--) TopBuffy = TopBuffy->prev;
  /* determine bottombuffy */
  BottomBuffy = CurBuffy;
  for(i = mailbox_position; i < count && BottomBuffy->next; i++)
    BottomBuffy = BottomBuffy->next;
}

static char * shortened_hierarchy(char * box) {
  int dots = 0;
  char * last_dot;
  int i,j;
  char * new_box;
  for (i=0;i<strlen(box);++i) {
    if (box[i] == '.') ++dots;
    else if (isupper (box[i])) 
      return (safe_strdup (box));
  }
  last_dot = strrchr(box,'.');
  if (last_dot) {
    ++last_dot;
    new_box = safe_malloc(strlen(last_dot)+2*dots+1);
    new_box[0] = box[0];
    for (i=1,j=1;i<strlen(box);++i) {
      if (box[i] == '.') {
        new_box[j++] = '.';
        new_box[j] = 0;
        if (&box[i+1] != last_dot) {
          new_box[j++] = box[i+1];
          new_box[j] = 0;
        } else {
          strcat(&new_box[j],last_dot);
          break;
        }
      }
    }
    return new_box;
  }
  return safe_strdup(box);
}

char *make_sidebar_entry(char *box, int size, int new, int flagged)
{
  int i = 0, dlen, max, shortened = 0;
  int offset;

  if (SidebarWidth > COLS)
    SidebarWidth = COLS;

  dlen = mutt_strlen(SidebarDelim);
  max = SidebarWidth - dlen - 1;

  safe_realloc(&entry, SidebarWidth + 1);
  entry[SidebarWidth] = 0;
  for (; i < SidebarWidth; entry[i++] = ' ' );
#if USE_IMAP
  if (ImapHomeNamespace && strlen(ImapHomeNamespace)>0) {
    if (strncmp(box,ImapHomeNamespace,strlen(ImapHomeNamespace))==0 && strcmp(box,ImapHomeNamespace)!=0) {
      box+=strlen(ImapHomeNamespace)+1;
    }
  }
#endif
  max -= quick_log10(size);
  if (new)
    max -= quick_log10(new) + 2;
  if (flagged > 0)
    max -= quick_log10(flagged) + 2;
  if (option(OPTSHORTENHIERARCHY) && mutt_strlen(box) > max) {
    box = shortened_hierarchy(box);
    shortened = 1;
  }
  i = strlen(box);
  strncpy( entry, box, i < SidebarWidth - dlen ? i :SidebarWidth - dlen);

  if ( new ) {
    if (flagged>0) {
      offset = SidebarWidth - 5 - quick_log10(size) - dlen - quick_log10(new) - quick_log10(flagged);
      if (offset<0) offset = 0;
      snprintf(entry + offset, SidebarWidth - dlen - offset + 1,
              "% d(%d)[%d]", size, new, flagged);
    } else {
      offset = SidebarWidth - 3 - quick_log10(size) - dlen - quick_log10(new);
      if (offset<0) offset = 0;
      snprintf(entry + offset, SidebarWidth - dlen - offset + 1,
              "% d(%d)", size, new);
    }
  } else {
    if (flagged>0) {
      offset = SidebarWidth - 3 - quick_log10(size) - dlen - quick_log10(flagged);
      if (offset<0) offset = 0;
      snprintf( entry + offset, SidebarWidth - dlen - offset + 1,
              "% d[%d]", size,flagged);
    } else {
      offset = SidebarWidth - 1 - quick_log10(size) - dlen;
      if (offset<0) offset = 0;
      snprintf( entry + offset, SidebarWidth - dlen - offset + 1,
              "% d", size);
    }
  }

  if (option(OPTSHORTENHIERARCHY) && shortened) {
    free(box);
  }
  return entry;
}

void set_curbuffy(char buf[LONG_STRING])
{
  BUFFY* tmp = CurBuffy = Incoming;

  if (!Incoming)
    return;

  while(1) {
    if(!strcmp(tmp->path, buf)) {
      CurBuffy = tmp;
      break;
    }

    if(tmp->next)
      tmp = tmp->next;
    else
      break;
  }
}

void set_buffystats (CONTEXT* Context)
{
  BUFFY* tmp = Incoming;
  if (!Context)
    return;
  while (tmp)
  {
    if (strcmp (tmp->path, Context->path) == 0)
    {
      tmp->new = Context->new;
      tmp->msg_unread = Context->unread;
      tmp->msgcount = Context->msgcount;
      tmp->msg_flagged = Context->flagged;
      break;
    }
    tmp = tmp->next;
  }
}

int draw_sidebar(int menu) {

  int lines = option(OPTHELP) ? 1 : 0;
  BUFFY *tmp;
  short delim_len = mutt_strlen (SidebarDelim);

  /* initialize first time */
  if(!initialized) {
    prev_show_value = option(OPTMBOXPANE);
    saveSidebarWidth = SidebarWidth;
    if(!option(OPTMBOXPANE)) SidebarWidth = 0;
    initialized = 1;
  }

  /* save or restore the value SidebarWidth */
  if(prev_show_value != option(OPTMBOXPANE)) {
    if(prev_show_value && !option(OPTMBOXPANE)) {
      saveSidebarWidth = SidebarWidth;
      SidebarWidth = 0;
    } else if(!prev_show_value && option(OPTMBOXPANE)) {
      SidebarWidth = saveSidebarWidth;
      /* after toggle: force recounting of all mail */
      mutt_buffy_check(2);
    }
    prev_show_value = option(OPTMBOXPANE);
  }

  if ( SidebarWidth == 0 ) return 0;

  /* draw the divider */
  /* SETCOLOR(MT_COLOR_STATUS); */
    SETCOLOR(MT_COLOR_SIDEBAR);
  for (lines = 1;
       lines < LINES-1-(menu != MENU_PAGER || option (OPTSTATUSONTOP)); lines++ ) {
    move(lines, SidebarWidth - delim_len);
    addstr (NONULL (SidebarDelim));
  }
  SETCOLOR(MT_COLOR_NORMAL);

  if ( Incoming == 0 ) return 0;
  lines = option(OPTHELP) ? 1 : 0; /* go back to the top */

  if ( CurBuffy == 0 ) CurBuffy = Incoming;
#if 0
  if ( known_lines != LINES || TopBuffy == 0 || BottomBuffy == 0 ) 
#endif
    calc_boundaries(menu);

  tmp = TopBuffy;

  for ( ; tmp && lines < LINES-1 - (menu != MENU_PAGER || option (OPTSTATUSONTOP)); tmp = tmp->next ) {
    if ( tmp == CurBuffy )
      SETCOLOR(MT_COLOR_INDICATOR);
    else if ( tmp->msg_flagged > 0 )
      SETCOLOR(MT_COLOR_FLAGGED);
    else if ( tmp->msg_unread > 0 )
      SETCOLOR(MT_COLOR_NEW);
    else
      SETCOLOR(MT_COLOR_NORMAL);

    move( lines, 0 );
    if (option(OPTSIDEBARNEWMAILONLY)) {
      if (tmp->msg_unread > 0) {
        if ( Context && !strcmp( tmp->path, Context->path ) ) {
          printw( "%.*s", SidebarWidth - delim_len,
                  make_sidebar_entry(basename(tmp->path),
                                     Context->msgcount, Context->unread, Context->flagged));
          tmp->msg_unread = Context->unread;
          tmp->msgcount = Context->msgcount;
          tmp->msg_flagged = Context->flagged;
        }
        else
          printw( "%.*s", SidebarWidth - delim_len,
                  make_sidebar_entry(basename(tmp->path),
                                     tmp->msgcount,tmp->msg_unread, tmp->msg_flagged));
        lines++;
      }
    } else {
      if ( Context && !strcmp( tmp->path, Context->path ) ) {
        printw( "%.*s", SidebarWidth - delim_len,
                make_sidebar_entry(basename(tmp->path),
                                   Context->msgcount, Context->unread, Context->flagged));
        tmp->msg_unread = Context->unread;
        tmp->msgcount = Context->msgcount;
        tmp->msg_flagged = Context->flagged;
      }
      else
        printw( "%.*s", SidebarWidth - delim_len,
                make_sidebar_entry(basename(tmp->path),
                                   tmp->msgcount,tmp->msg_unread, tmp->msg_flagged));
      lines++;
    }
  }
  SETCOLOR(MT_COLOR_NORMAL);
  for ( ; lines < LINES - 1 - (menu != MENU_PAGER || option (OPTSTATUSONTOP)); lines++ ) {
    int i = 0;
    move( lines, 0 );
    for ( ; i < SidebarWidth - delim_len; i++ )
      addch(' ');
  }
  return 0;
}

BUFFY * exist_next_new()
{
	BUFFY *tmp = CurBuffy;
	if(tmp == NULL) return NULL;
	while (tmp->next != NULL)
	{
		tmp = tmp->next;
		if(tmp->msg_unread) return tmp;
	}
	return NULL;
}

BUFFY * exist_prev_new()
{
	BUFFY *tmp = CurBuffy;
	if(tmp == NULL) return NULL;
	while (tmp->prev != NULL)
	{
		tmp = tmp->prev;
		if(tmp->msg_unread) return tmp;
	}
	return NULL;
}     


void scroll_sidebar(int op, int menu)
{
  BUFFY *tmp;

  if(!SidebarWidth) return;
  if(!CurBuffy) return;

  switch (op) {
    case OP_SIDEBAR_NEXT:
      if (!option(OPTSIDEBARNEWMAILONLY)) {
        if ( CurBuffy->next == NULL ) {
          mutt_error (_("You are on the last mailbox."));
          return;
        }
        CurBuffy = CurBuffy->next;
        break;
      } /* the fall-through is intentional */
    case OP_SIDEBAR_NEXT_NEW:
      if ( (tmp = exist_next_new()) == NULL) {
        mutt_error (_("No next mailboxes with new mail."));
        return;
      }
      else CurBuffy = tmp;
      break;
    case OP_SIDEBAR_PREV:
      if (!option(OPTSIDEBARNEWMAILONLY)) {
        if ( CurBuffy->prev == NULL ) {
          mutt_error (_("You are on the first mailbox."));
          return;
        }
        CurBuffy = CurBuffy->prev;
       break;
     } /* the fall-through is intentional */
    case OP_SIDEBAR_PREV_NEW:
      if ( (tmp = exist_prev_new()) == NULL) {
        mutt_error (_("No previous mailbox with new mail."));
        return;
      }
      else CurBuffy = tmp;
      break;
     
    case OP_SIDEBAR_SCROLL_UP:
      CurBuffy = TopBuffy;
      if ( CurBuffy != Incoming ) {
        calc_boundaries(menu);
        CurBuffy = CurBuffy->prev;
      }
      break;
    case OP_SIDEBAR_SCROLL_DOWN:
      CurBuffy = BottomBuffy;
      if ( CurBuffy->next ) {
        calc_boundaries(menu);
        CurBuffy = CurBuffy->next;
      }
      break;
    default:
      return;
  }
  calc_boundaries(menu);
  draw_sidebar(menu);
}
