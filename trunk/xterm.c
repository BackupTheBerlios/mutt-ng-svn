/*
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */
#include <stdio.h>
#include "xterm.h"
#include "lib/str.h"

void mutt_xterm_set_title (char *title) {
  fputs ("\033]2;", stdout);
  fputs (NONULL (title), stdout);
  fputs ("\007", stdout);
  fflush (stdout);
}

void mutt_xterm_set_icon (char *name) {
  fputs ("\033]1;", stdout);
  fputs (NONULL (name), stdout);
  fputs ("\007", stdout);
  fflush (stdout);
}
