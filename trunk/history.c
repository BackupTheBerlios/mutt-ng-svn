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
#include "history.h"
#include "lib/mem.h"

/* global vars used for the string-history routines */

struct history {
  char **hist;
  short cur;
  short last;
};

static struct history History[HC_LAST];
static int OldSize = 0;

static void init_history (struct history *h)
{
  int i;

  if (OldSize) {
    if (h->hist) {
      for (i = 0; i < OldSize; i++)
        mem_free (&h->hist[i]);
      mem_free (&h->hist);
    }
  }

  if (HistSize)
    h->hist = mem_calloc (HistSize, sizeof (char *));

  h->cur = 0;
  h->last = 0;
}

void mutt_init_history (void)
{
  history_class_t hclass;

  if (HistSize == OldSize)
    return;

  for (hclass = HC_FIRST; hclass < HC_LAST; hclass++)
    init_history (&History[hclass]);

  OldSize = HistSize;
}

void mutt_history_add (history_class_t hclass, const char *s)
{
  int prev;
  struct history *h = &History[hclass];

  if (!HistSize)
    return;                     /* disabled */

  if (*s) {
    prev = h->last - 1;
    if (prev < 0)
      prev = HistSize - 1;
    if (!h->hist[prev] || str_cmp (h->hist[prev], s) != 0) {
      str_replace (&h->hist[h->last++], s);
      if (h->last > HistSize - 1)
        h->last = 0;
    }
  }
  h->cur = h->last;             /* reset to the last entry */
}

char *mutt_history_next (history_class_t hclass)
{
  int next;
  struct history *h = &History[hclass];

  if (!HistSize)
    return ("");                /* disabled */

  next = h->cur + 1;
  if (next > HistSize - 1)
    next = 0;
  h->cur = h->hist[next] ? next : 0;
  return (h->hist[h->cur] ? h->hist[h->cur] : "");
}

char *mutt_history_prev (history_class_t hclass)
{
  int prev;
  struct history *h = &History[hclass];

  if (!HistSize)
    return ("");                /* disabled */

  prev = h->cur - 1;
  if (prev < 0) {
    prev = HistSize - 1;
    while (prev > 0 && h->hist[prev] == NULL)
      prev--;
  }
  if (h->hist[prev])
    h->cur = prev;
  return (h->hist[h->cur] ? h->hist[h->cur] : "");
}
