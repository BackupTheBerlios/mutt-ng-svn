/**
 * @ingroup core
 */
/**
 * @file core/exit.c
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Fatal error handling implementation
 */
#include <stdlib.h>
#include <stdio.h>

#include "exit.h"
#include "str.h"
#include "intl.h"

/**
 * @bug Remove this as port of rewrite.
 */
void exit_fatal (const char* func, const char* msg, int line, 
                 const char* fname, int code) {
  fprintf (stderr, _("Fatal error in function '%s' called from "
                     "file '%s', line '%d': %s\n"
                     "(please report this error to "
                     "<mutt-ng-devel@lists.berlios.de>\n"),
           NONULL(func), NONULL(fname), line, NONULL(msg));
  exit (code);
}
