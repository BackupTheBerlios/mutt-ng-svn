
#include <stdlib.h>
#include <stdio.h>

#include "exit.h"
#include "str.h"
#include "intl.h"

/* XXX remove after modularization*/
/*extern void mutt_endwin (void*);*/

void exit_fatal (const char* func, const char* msg, int line, 
                 const char* fname, int code) {
/*  mutt_endwin (NULL);*/
  fprintf (stderr, _("Fatal error in function '%s' called from "
                     "file '%s', line '%d': %s\n"
                     "(please report this error to "
                     "<mutt-ng-devel@lists.berlios.de>\n"),
           NONULL(func), NONULL(fname), line, NONULL(msg));
  exit (code);
}
