/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
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

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#else
# ifdef HAVE_IOCTL_H
# include <ioctl.h>
# endif
#endif

/* this routine should be called after receiving SIGWINCH */
void mutt_resize_screen (void)
{
  char *cp;
  int fd;
  struct winsize w;

#ifdef HAVE_RESIZETERM
  int SLtt_Screen_Rows, SLtt_Screen_Cols;
#endif

  SLtt_Screen_Rows = -1;
  SLtt_Screen_Cols = -1;
  if ((fd = open ("/dev/tty", O_RDONLY)) != -1) {
    if (ioctl (fd, TIOCGWINSZ, &w) != -1) {
      SLtt_Screen_Rows = w.ws_row;
      SLtt_Screen_Cols = w.ws_col;
    }
    close (fd);
  }
  if (SLtt_Screen_Rows <= 0) {
    if ((cp = getenv ("LINES")) != NULL) {
      SLtt_Screen_Rows = atoi (cp);
    }
    else
      SLtt_Screen_Rows = 24;
  }
  if (SLtt_Screen_Cols <= 0) {
    if ((cp = getenv ("COLUMNS")) != NULL)
      SLtt_Screen_Cols = atoi (cp);
    else
      SLtt_Screen_Cols = 80;
  }
#ifdef USE_SLANG_CURSES
  delwin (stdscr);
  SLsmg_reset_smg ();
  SLsmg_init_smg ();
  stdscr = newwin (0, 0, 0, 0);
  keypad (stdscr, TRUE);
#else
  resizeterm (SLtt_Screen_Rows, SLtt_Screen_Cols);
#endif
}
