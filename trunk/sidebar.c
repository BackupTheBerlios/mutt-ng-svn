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
#include <stdbool.h>
#include <ctype.h>

/*BUFFY *CurBuffy = 0;*/
static BUFFY *TopBuffy = 0;
static BUFFY *BottomBuffy = 0;
static int known_lines = 0;
static bool initialized = false;
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

static int cur_is_hidden (int maxline)
{
  int l = 0, seen = 0;
  BUFFY* tmp = TopBuffy;
  if (!CurBuffy)
    return (0);
  while (tmp && l < maxline && !seen)
  {
    if (strcmp (tmp->path, CurBuffy->path) == 0)
      seen = 1;
    else
      tmp = tmp->next;
    l++;
  }
  return (seen == 0 || l == maxline);
}

void calc_boundaries (int menu)
{
  BUFFY *tmp = Incoming;

  if ( known_lines != LINES ) {
    TopBuffy = BottomBuffy = 0;
    known_lines = LINES;
  }
  for ( ; tmp->next != 0; tmp = tmp->next )
    tmp->next->prev = tmp;

  if ( TopBuffy == 0 && BottomBuffy == 0 )
    TopBuffy = Incoming;
  if ( BottomBuffy == 0 ) {
    int count = LINES - 2 - (menu != MENU_PAGER || option (OPTSTATUSONTOP));
    BottomBuffy = TopBuffy;
    while ( --count && BottomBuffy->next )
      BottomBuffy = BottomBuffy->next;
  }
  else if ( TopBuffy == CurBuffy->next ) {
    int count = LINES - 2 - (menu != MENU_PAGER);
    BottomBuffy = CurBuffy;
    tmp = BottomBuffy;
    while ( --count && tmp->prev)
      tmp = tmp->prev;
    TopBuffy = tmp;
  }
  else if ( BottomBuffy == CurBuffy->prev ) {
    int count = LINES - 2 - (menu != MENU_PAGER);
    TopBuffy = CurBuffy;
    tmp = TopBuffy;
    while ( --count && tmp->next )
      tmp = tmp->next;
    BottomBuffy = tmp;
  }
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

char *make_sidebar_entry(char *box, int size, int new)
{
  char *c;
  int i = 0, dlen = mutt_strlen (SidebarDelim);

  c = realloc(entry, SidebarWidth + 1);
  if ( c ) entry = c;
  entry[SidebarWidth] = 0;
  for (; i < SidebarWidth; entry[i++] = ' ' );
#if USE_IMAP
  if (ImapHomeNamespace && strlen(ImapHomeNamespace)>0) {
    if (strncmp(box,ImapHomeNamespace,strlen(ImapHomeNamespace))==0 && strcmp(box,ImapHomeNamespace)!=0) {
      box+=strlen(ImapHomeNamespace)+1;
    }
  }
#endif
  if (option(OPTSHORTENHIERARCHY)) {
    box = shortened_hierarchy(box);
  }
  i = strlen(box);
  strncpy( entry, box, i < SidebarWidth - dlen ? i :SidebarWidth - dlen);

  if ( new ) 
    sprintf(entry + SidebarWidth - 3 - quick_log10(size) - dlen - quick_log10(new),
            "% d(%d)", size, new);
  else
    sprintf( entry + SidebarWidth - 1 - quick_log10(size) - dlen, "% d", size);
  if (option(OPTSHORTENHIERARCHY)) {
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
  while (tmp)
  {
    if (strcmp (tmp->path, Context->path) == 0)
    {
      tmp->msg_unread = Context->unread;
      tmp->msgcount = Context->msgcount;
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
    initialized = true;
  }

  /* save or restore the value SidebarWidth */
  if(prev_show_value != option(OPTMBOXPANE)) {
    if(prev_show_value && !option(OPTMBOXPANE)) {
      saveSidebarWidth = SidebarWidth;
      SidebarWidth = 0;
    } else if(!prev_show_value && option(OPTMBOXPANE)) {
      SidebarWidth = saveSidebarWidth;
    }
    prev_show_value = option(OPTMBOXPANE);
  }

  if ( SidebarWidth == 0 ) return 0;

  /* draw the divider */
  SETCOLOR(MT_COLOR_STATUS);
  for (lines = option (OPTSTATUSONTOP) ? 0 : 1; 
       lines < LINES-1-(menu != MENU_PAGER || option (OPTSTATUSONTOP)); lines++ ) {
    move(lines, SidebarWidth - delim_len);
    addstr (NONULL (SidebarDelim));
  }
  SETCOLOR(MT_COLOR_NORMAL);

  if ( Incoming == 0 ) return 0;
  lines = option(OPTHELP) ? 1 : 0; /* go back to the top */

  if (cur_is_hidden (LINES-1-(menu != MENU_PAGER)))
    CurBuffy = TopBuffy;

  if ( known_lines != LINES || TopBuffy == 0 || BottomBuffy == 0 ) 
    calc_boundaries(menu);
  if ( CurBuffy == 0 ) CurBuffy = Incoming;

  tmp = TopBuffy;

  for ( ; tmp && lines < LINES-1 - (menu != MENU_PAGER || option (OPTSTATUSONTOP)); tmp = tmp->next ) {
    if ( tmp == CurBuffy )
      SETCOLOR(MT_COLOR_INDICATOR);
    else if ( tmp->msg_unread > 0 )
      SETCOLOR(MT_COLOR_NEW);
    else
      SETCOLOR(MT_COLOR_NORMAL);

    move( lines, 0 );
    if ( Context && !strcmp( tmp->path, Context->path ) ) {
      printw( "%.*s", SidebarWidth - delim_len,
              make_sidebar_entry(basename(tmp->path),
                                 Context->msgcount, Context->unread));
      tmp->msg_unread = Context->unread;
      tmp->msgcount = Context->msgcount;
    }
    else
      printw( "%.*s", SidebarWidth - delim_len,
              make_sidebar_entry(basename(tmp->path),
                                 tmp->msgcount,tmp->msg_unread));
    lines++;
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

void scroll_sidebar(int op, int menu)
{
        if(!SidebarWidth) return;
        if(!CurBuffy) return;

  switch (op) {
    case OP_SIDEBAR_NEXT:
      if ( CurBuffy->next == NULL ) return;
      CurBuffy = CurBuffy->next;
      break;
    case OP_SIDEBAR_PREV:
      if ( CurBuffy == Incoming ) return;
      {
        BUFFY *tmp = Incoming;
        while ( tmp->next && strcmp(tmp->next->path, CurBuffy->path) ) tmp = tmp->next;
        CurBuffy = tmp;
      }
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
