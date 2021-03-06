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
#include "sort.h"
#include "buffer.h"

#include "lib/mem.h"
#include "lib/intl.h"

#include <string.h>
#include <stdlib.h>

typedef struct score_t {
  char *str;
  pattern_t *pat;
  int val;
  int exact;                    /* if this rule matches, don't evaluate any more */
  struct score_t *next;
} SCORE;

SCORE *Score = NULL;

void mutt_check_rescore (CONTEXT * ctx)
{
  int i;

  if (option (OPTNEEDRESCORE) && option (OPTSCORE)) {
    if ((Sort & SORT_MASK) == SORT_SCORE ||
        (SortAux & SORT_MASK) == SORT_SCORE) {
      set_option (OPTNEEDRESORT);
      if ((Sort & SORT_MASK) == SORT_THREADS)
        set_option (OPTSORTSUBTHREADS);
    }

    /* must redraw the index since the user might have %N in it */
    set_option (OPTFORCEREDRAWINDEX);
    set_option (OPTFORCEREDRAWPAGER);

    for (i = 0; ctx && i < ctx->msgcount; i++) {
      mutt_score_message (ctx, ctx->hdrs[i], 1);
      ctx->hdrs[i]->pair = 0;
    }
  }
  unset_option (OPTNEEDRESCORE);
}

int mutt_parse_score (BUFFER * buf, BUFFER * s, unsigned long data,
                      BUFFER * err)
{
  SCORE *ptr, *last;
  char *pattern, *pc;
  struct pattern_t *pat;

  mutt_extract_token (buf, s, 0);
  if (!MoreArgs (s)) {
    strfcpy (err->data, _("score: too few arguments"), err->dsize);
    return (-1);
  }
  pattern = buf->data;
  memset (buf, 0, sizeof (BUFFER));
  mutt_extract_token (buf, s, 0);
  if (MoreArgs (s)) {
    mem_free (&pattern);
    strfcpy (err->data, _("score: too many arguments"), err->dsize);
    return (-1);
  }

  /* look for an existing entry and update the value, else add it to the end
     of the list */
  for (ptr = Score, last = NULL; ptr; last = ptr, ptr = ptr->next)
    if (str_cmp (pattern, ptr->str) == 0)
      break;
  if (!ptr) {
    if ((pat = mutt_pattern_comp (pattern, 0, err)) == NULL) {
      mem_free (&pattern);
      return (-1);
    }
    ptr = mem_calloc (1, sizeof (SCORE));
    if (last)
      last->next = ptr;
    else
      Score = ptr;
    ptr->pat = pat;
    ptr->str = pattern;
  }
  pc = buf->data;
  if (*pc == '=') {
    ptr->exact = 1;
    pc++;
  }
  ptr->val = atoi (pc);
  set_option (OPTNEEDRESCORE);
  return 0;
}

void mutt_score_message (CONTEXT * ctx, HEADER * hdr, int upd_ctx)
{
  SCORE *tmp;

  hdr->score = 0;               /* in case of re-scoring */
  for (tmp = Score; tmp; tmp = tmp->next) {
    if (mutt_pattern_exec (tmp->pat, M_MATCH_FULL_ADDRESS, NULL, hdr) > 0) {
      if (tmp->exact || tmp->val == 9999 || tmp->val == -9999) {
        hdr->score = tmp->val;
        break;
      }
      hdr->score += tmp->val;
    }
  }
  if (hdr->score < 0)
    hdr->score = 0;

  if (hdr->score <= ScoreThresholdDelete)
    _mutt_set_flag (ctx, hdr, M_DELETE, 1, upd_ctx);
  if (hdr->score <= ScoreThresholdRead)
    _mutt_set_flag (ctx, hdr, M_READ, 1, upd_ctx);
  if (hdr->score >= ScoreThresholdFlag)
    _mutt_set_flag (ctx, hdr, M_FLAG, 1, upd_ctx);
}

int mutt_parse_unscore (BUFFER * buf, BUFFER * s, unsigned long data,
                        BUFFER * err)
{
  SCORE *tmp, *last = NULL;

  while (MoreArgs (s)) {
    mutt_extract_token (buf, s, 0);
    if (!str_cmp ("*", buf->data)) {
      for (tmp = Score; tmp;) {
        last = tmp;
        tmp = tmp->next;
        mutt_pattern_free (&last->pat);
        mem_free (&last);
      }
      Score = NULL;
    }
    else {
      for (tmp = Score; tmp; last = tmp, tmp = tmp->next) {
        if (!str_cmp (buf->data, tmp->str)) {
          if (last)
            last->next = tmp->next;
          else
            Score = tmp->next;
          mutt_pattern_free (&tmp->pat);
          mem_free (&tmp);
          /* there should only be one score per pattern, so we can stop here */
          break;
        }
      }
    }
  }
  set_option (OPTNEEDRESCORE);
  return 0;
}
