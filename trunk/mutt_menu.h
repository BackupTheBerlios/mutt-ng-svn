/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

/*
 * This file is named mutt_menu.h so it doesn't collide with ncurses menu.h
 */

#include "keymap.h"
#include "mutt_regex.h"

#define REDRAW_INDEX		(1)
#define REDRAW_MOTION		(1<<1)
#define REDRAW_MOTION_RESYNCH	(1<<2)
#define REDRAW_CURRENT		(1<<3)
#define REDRAW_STATUS		(1<<4)
#define REDRAW_FULL		(1<<5)
#define REDRAW_BODY		(1<<6)
#define REDRAW_SIGWINCH		(1<<7)
#define REDRAW_SIDEBAR		(1<<8)

#define M_MODEFMT "-- Mutt-ng: %s"

typedef struct menu_t {
  char *title;                  /* the title of this menu */
  char *help;                   /* quickref for the current menu */
  void *data;                   /* extra data for the current menu */
  int current;                  /* current entry */
  int max;                      /* the number of entries in the menu */
  int redraw;                   /* when to redraw the screen */
  int menu;                     /* menu definition for keymap entries. */
  int offset;                   /* which screen row to start the index */
  int pagelen;                  /* number of entries per screen */
  int tagprefix;

  /* Setting dialog != NULL overrides normal menu behaviour. 
   * In dialog mode menubar is hidden and prompt keys are checked before
   * normal menu movement keys. This can cause problems with scrolling, if 
   * prompt keys override movement keys.
   */
  char **dialog;                /* dialog lines themselves */
  char *prompt;                 /* prompt for user, similar to mutt_multi_choice */
  char *keys;                   /* keys used in the prompt */

  /* callback to generate an index line for the requested element */
  void (*make_entry) (char *, size_t, struct menu_t *, int);

  /* how to search the menu */
  int (*search) (struct menu_t *, regex_t * re, int n);

  int (*tag) (struct menu_t *, int i, int m);

  /* color pair to be used for the requested element 
   * (default function returns ColorDefs[MT_COLOR_NORMAL])
   */
  int (*color) (int i);

  /* the following are used only by mutt_menuLoop() */
  int top;                      /* entry that is the top of the current page */
  int oldcurrent;               /* for driver use only. */
  char *searchBuf;              /* last search pattern */
  int searchDir;                /* direction of search */
  int tagged;                   /* number of tagged entries */
} MUTTMENU;

void menu_jump (MUTTMENU *);
void menu_redraw_full (MUTTMENU *);
void menu_redraw_index (MUTTMENU *);
void menu_redraw_status (MUTTMENU *);
void menu_redraw_motion (MUTTMENU *);
void menu_redraw_current (MUTTMENU *);
int menu_redraw (MUTTMENU *);
void menu_first_entry (MUTTMENU *);
void menu_last_entry (MUTTMENU *);
void menu_top_page (MUTTMENU *);
void menu_bottom_page (MUTTMENU *);
void menu_middle_page (MUTTMENU *);
void menu_next_page (MUTTMENU *);
void menu_prev_page (MUTTMENU *);
void menu_next_line (MUTTMENU *);
void menu_prev_line (MUTTMENU *);
void menu_half_up (MUTTMENU *);
void menu_half_down (MUTTMENU *);
void menu_current_top (MUTTMENU *);
void menu_current_middle (MUTTMENU *);
void menu_current_bottom (MUTTMENU *);
void menu_check_recenter (MUTTMENU *);
void menu_status_line (char *, size_t, MUTTMENU *, const char *);

MUTTMENU *mutt_new_menu (void);
void mutt_menuDestroy (MUTTMENU **);
int mutt_menuLoop (MUTTMENU *);

/* used in both the index and pager index to make an entry. */
void index_make_entry (char *, size_t, struct menu_t *, int);
int index_color (int);
