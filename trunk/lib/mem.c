/*
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#include <stdlib.h>

#include "mem.h"
#include "exit.h"
#include "intl.h"

void *_mem_calloc (size_t nmemb, size_t size, int line, const char* fname) {
  void *p;

  if (!nmemb || !size)
    return NULL;

  if (((size_t) - 1) / nmemb <= size) {
    exit_fatal ("mem_calloc", _("Integer overflow -- can't allocate memory!"),
                line, fname, 1);
    return (NULL);
  }

  if (!(p = calloc (nmemb, size))) {
    exit_fatal ("mem_calloc", _("Out of memory!"), line, fname, 1);
    return (NULL);
  }
  return p;
}

void *_mem_malloc (size_t siz, int line, const char* fname) {
  void *p;

  if (siz == 0)
    return 0;
  if ((p = (void *) malloc (siz)) == 0) {       /* __MEM_CHECKED__ */
    exit_fatal ("mem_malloc", _("Out of memory!"), line, fname, 1);
    return (NULL);
  }
  return (p);
}

void _mem_realloc (void *ptr, size_t siz, int line, const char* fname) {
  void *r;
  void **p = (void **) ptr;

  if (siz == 0) {
    if (*p) {
      free (*p);                /* __MEM_CHECKED__ */
      *p = NULL;
    }
    return;
  }

  if (*p)
    r = (void *) realloc (*p, siz);     /* __MEM_CHECKED__ */
  else {
    /* realloc(NULL, nbytes) doesn't seem to work under SunOS 4.1.x  --- __MEM_CHECKED__ */
    r = (void *) malloc (siz);  /* __MEM_CHECKED__ */
  }

  if (!r)
    exit_fatal ("mem_realloc", _("Out of memory!"), line, fname, 1);

  *p = r;
}

void _mem_free (void *ptr) {
  void **p = (void **) ptr;

  if (*p) {
    free (*p);                  /* __MEM_CHECKED__ */
    *p = 0;
  }
}
