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
#include "mutt_menu.h"
#include "sort.h"
#include "mx.h"
#include "sidebar.h"

#ifdef USE_IMAP
#include "imap_private.h"
#endif

#include "lib/intl.h"

void _mutt_set_flag (CONTEXT * ctx, HEADER * h, int flag, int bf, int upd_ctx)
{
  int changed = h->changed;
  int deleted = ctx->deleted;
  int tagged = ctx->tagged;

  if (ctx->readonly && flag != M_TAG)
    return;                     /* don't modify anything if we are read-only */

  switch (flag) {
  case M_DELETE:

#ifdef USE_IMAP
    if (ctx && ctx->magic == M_IMAP)
      if (mutt_bit_isset (((IMAP_DATA *) ctx->data)->capabilities, ACL)
          && !mutt_bit_isset (((IMAP_DATA *) ctx->data)->rights,
                              IMAP_ACL_DELETE))
        return;
#endif

    if (bf) {
      if (!h->deleted && !ctx->readonly) {
        h->deleted = 1;
        if (upd_ctx)
          ctx->deleted++;
#ifdef USE_IMAP
        /* deleted messages aren't treated as changed elsewhere so that the
         * purge-on-sync option works correctly. This isn't applicable here */
        if (ctx && ctx->magic == M_IMAP) {
          h->changed = 1;
          if (upd_ctx)
            ctx->changed = 1;
        }
#endif
      }
    }
    else if (h->deleted) {
      h->deleted = 0;
      if (upd_ctx) {
        ctx->deleted--;
        if (h->appended)
          ctx->appended--;
      }
      h->appended = 0;          /* when undeleting, also reset the appended flag */
#ifdef USE_IMAP
      /* see my comment above */
      if (ctx->magic == M_IMAP) {
        h->changed = 1;
        if (upd_ctx)
          ctx->changed = 1;
      }
#endif
      /* 
       * If the user undeletes a message which is marked as
       * "trash" in the maildir folder on disk, the folder has
       * been changed, and is marked accordingly.  However, we do
       * _not_ mark the message itself changed, because trashing
       * is checked in specific code in the maildir folder
       * driver. 
       */
      if (ctx->magic == M_MAILDIR && upd_ctx && h->trash)
        ctx->changed = 1;
    }
    break;

  case M_APPENDED:
    if (bf) {
      if (!h->appended) {
        h->appended = 1;
        if (upd_ctx)
          ctx->appended++;
      }
    }
    break;

  case M_PURGED:
    if (bf) {
      if (!h->purged)
        h->purged = 1;
    }
    else if (h->purged)
      h->purged = 0;
    break;

  case M_NEW:

#ifdef USE_IMAP
    if (ctx && ctx->magic == M_IMAP)
      if (mutt_bit_isset (((IMAP_DATA *) ctx->data)->capabilities, ACL)
          && !mutt_bit_isset (((IMAP_DATA *) ctx->data)->rights,
                              IMAP_ACL_SEEN))
        return;
#endif

    if (bf) {
      if (h->read || h->old) {
        h->old = 0;
        if (upd_ctx)
          ctx->new++;
        if (h->read) {
          h->read = 0;
          if (upd_ctx)
            ctx->unread++;
        }
        h->changed = 1;
        if (upd_ctx)
          ctx->changed = 1;
      }
    }
    else if (!h->read) {
      if (!h->old)
        if (upd_ctx)
          ctx->new--;
      h->read = 1;
      if (upd_ctx)
        ctx->unread--;
      h->changed = 1;
      if (upd_ctx)
        ctx->changed = 1;
    }
    break;

  case M_OLD:

#ifdef USE_IMAP
    if (ctx && ctx->magic == M_IMAP)
      if (mutt_bit_isset (((IMAP_DATA *) ctx->data)->capabilities, ACL)
          && !mutt_bit_isset (((IMAP_DATA *) ctx->data)->rights,
                              IMAP_ACL_SEEN))
        return;
#endif

    if (bf) {
      if (!h->old) {
        h->old = 1;
        if (!h->read)
          if (upd_ctx)
            ctx->new--;
        h->changed = 1;
        if (upd_ctx)
          ctx->changed = 1;
      }
    }
    else if (h->old) {
      h->old = 0;
      if (!h->read)
        if (upd_ctx)
          ctx->new++;
      h->changed = 1;
      if (upd_ctx)
        ctx->changed = 1;
    }
    break;

  case M_READ:

#ifdef USE_IMAP
    if (ctx && ctx->magic == M_IMAP)
      if (mutt_bit_isset (((IMAP_DATA *) ctx->data)->capabilities, ACL)
          && !mutt_bit_isset (((IMAP_DATA *) ctx->data)->rights,
                              IMAP_ACL_SEEN))
        return;
#endif

    if (bf) {
      if (!h->read) {
        h->read = 1;
        if (upd_ctx)
          ctx->unread--;
        if (!h->old)
          if (upd_ctx)
            ctx->new--;
        h->changed = 1;
        if (upd_ctx)
          ctx->changed = 1;
      }
    }
    else if (h->read) {
      h->read = 0;
      if (upd_ctx)
        ctx->unread++;
      if (!h->old)
        if (upd_ctx)
          ctx->new++;
      h->changed = 1;
      if (upd_ctx)
        ctx->changed = 1;
    }
    break;

  case M_REPLIED:

#ifdef USE_IMAP
    if (ctx && ctx->magic == M_IMAP)
      if (mutt_bit_isset (((IMAP_DATA *) ctx->data)->capabilities, ACL)
          && !mutt_bit_isset (((IMAP_DATA *) ctx->data)->rights,
                              IMAP_ACL_WRITE))
        return;
#endif

    if (bf) {
      if (!h->replied) {
        h->replied = 1;
        if (!h->read) {
          h->read = 1;
          if (upd_ctx)
            ctx->unread--;
          if (!h->old)
            if (upd_ctx)
              ctx->new--;
        }
        h->changed = 1;
        if (upd_ctx)
          ctx->changed = 1;
      }
    }
    else if (h->replied) {
      h->replied = 0;
      h->changed = 1;
      if (upd_ctx)
        ctx->changed = 1;
    }
    break;

  case M_FLAG:

#ifdef USE_IMAP
    if (ctx && ctx->magic == M_IMAP)
      if (mutt_bit_isset (((IMAP_DATA *) ctx->data)->capabilities, ACL)
          && !mutt_bit_isset (((IMAP_DATA *) ctx->data)->rights,
                              IMAP_ACL_WRITE))
        return;
#endif

    if (bf) {
      if (!h->flagged) {
        h->flagged = bf;
        if (upd_ctx)
          ctx->flagged++;
        h->changed = 1;
        if (upd_ctx)
          ctx->changed = 1;
      }
    }
    else if (h->flagged) {
      h->flagged = 0;
      if (upd_ctx)
        ctx->flagged--;
      h->changed = 1;
      if (upd_ctx)
        ctx->changed = 1;
    }
    break;

  case M_TAG:
    if (bf) {
      if (!h->tagged) {
        h->tagged = 1;
        if (upd_ctx)
          ctx->tagged++;
      }
    }
    else if (h->tagged) {
      h->tagged = 0;
      if (upd_ctx)
        ctx->tagged--;
    }
    break;
  }

  mutt_set_header_color (ctx, h);

  /* if the message status has changed, we need to invalidate the cached
   * search results so that any future search will match the current status
   * of this message and not what it was at the time it was last searched.
   */
  if (h->searched
      && (changed != h->changed || deleted != ctx->deleted
          || tagged != ctx->tagged))
    h->searched = 0;
  sidebar_draw (0);
}

void mutt_tag_set_flag (int flag, int bf)
{
  int j;

  for (j = 0; j < Context->vcount; j++)
    if (Context->hdrs[Context->v2r[j]]->tagged)
      mutt_set_flag (Context, Context->hdrs[Context->v2r[j]], flag, bf);
}
int mutt_thread_set_flag (HEADER * hdr, int flag, int bf, int subthread)
{
  THREAD *start, *cur = hdr->thread;

  if ((Sort & SORT_MASK) != SORT_THREADS) {
    mutt_error _("Threading is not enabled.");

    return (-1);
  }

  if (!subthread)
    while (cur->parent)
      cur = cur->parent;
  start = cur;

  if (cur->message)
    mutt_set_flag (Context, cur->message, flag, bf);

  if ((cur = cur->child) == NULL)
    return (0);

  FOREVER {
    if (cur->message)
      mutt_set_flag (Context, cur->message, flag, bf);

    if (cur->child)
      cur = cur->child;
    else if (cur->next)
      cur = cur->next;
    else {
      while (!cur->next) {
        cur = cur->parent;
        if (cur == start)
          return (0);
      }
      cur = cur->next;
    }
  }
  /* not reached */
}

int mutt_change_flag (HEADER * h, int bf)
{
  int i, flag;
  event_t event;

  mvprintw (LINES - 1, 0, "%s? (D/N/O/r/*/!): ",
            bf ? _("Set flag") : _("Clear flag"));
  clrtoeol ();

  event = mutt_getch ();
  i = event.ch;
  if (i == -1) {
    CLEARLINE (LINES - 1);
    return (-1);
  }

  CLEARLINE (LINES - 1);

  switch (i) {
  case 'd':
  case 'D':
    flag = M_DELETE;
    break;

  case 'N':
  case 'n':
    flag = M_NEW;
    break;

  case 'o':
  case 'O':
    if (h)
      mutt_set_flag (Context, h, M_READ, !bf);
    else
      mutt_tag_set_flag (M_READ, !bf);
    flag = M_OLD;
    break;

  case 'r':
  case 'R':
    flag = M_REPLIED;
    break;

  case '*':
    flag = M_TAG;
    break;

  case '!':
    flag = M_FLAG;
    break;

  default:
    BEEP ();
    return (-1);
  }

  if (h)
    mutt_set_flag (Context, h, flag, bf);
  else
    mutt_tag_set_flag (flag, bf);

  return 0;
}
