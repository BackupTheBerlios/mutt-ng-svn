/*
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#include "rx.h"

#include "mem.h"
#include "str.h"

rx_t *rx_compile (const char *s, int flags) {
  rx_t *pp = safe_calloc (sizeof (rx_t), 1);

  pp->pattern = safe_strdup (s);
  pp->rx = safe_calloc (sizeof (regex_t), 1);
  if (REGCOMP(pp->rx, NONULL (s), flags) != 0)
    rx_free (&pp);

  return pp;
}

void rx_free (rx_t** p) {
  FREE(&(*p)->pattern);
  regfree ((*p)->rx);
  FREE(&(*p)->rx);
  FREE(p);
}

int rx_compare (const rx_t* r1, const rx_t* r2) {
  return (mutt_strcmp (r1->pattern, r2->pattern));
}

int rx_list_match (list2_t* l, const char* pat) {
  int i = 0;
  if (!pat || list_empty(l))
    return (0);
  for (i = 0; i < l->length; i++)
    if (REGEXEC(((rx_t*) l->data[i])->rx, pat) == 0)
      return (1);
  return (0);
}

int rx_lookup (list2_t* l, const char* pat) {
  int i = 0;
  if (!pat || !*pat || list_empty(l))
    return (-1);
  for (i = 0; i < l->length; i++)
    if (mutt_strcmp (((rx_t*) l->data[i])->pattern, pat) == 0)
      return (i);
  return (-1);
}
