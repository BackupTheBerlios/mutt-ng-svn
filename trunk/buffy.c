/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 *
 * Parts were written/modified by:
 * Rocco Rutte <pdmef@cs.tu-berlin.de>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "mutt.h"
#include "buffy.h"
#include "mx.h"
#include "mh.h"
#include "sidebar.h"

#include "mutt_curses.h"

#ifdef USE_IMAP
#include "imap.h"
#endif

#include "lib/mem.h"
#include "lib/intl.h"

#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <utime.h>
#include <ctype.h>
#include <unistd.h>

#include <stdio.h>

static time_t BuffyTime = 0;    /* last time we started checking for mail */

#ifdef USE_IMAP
static time_t ImapBuffyTime = 0;        /* last time we started checking for mail */
#endif
static short BuffyCount = 0;    /* how many boxes with new mail */
static short BuffyNotify = 0;   /* # of unnotified new boxes */

#ifdef BUFFY_SIZE

/* Find the last message in the file. 
 * upon success return 0. If no message found - return -1 */

static int fseek_last_message (FILE * f)
{
  long int pos;
  char buffer[BUFSIZ + 9];      /* 7 for "\n\nFrom " */
  int bytes_read;
  int i;                        /* Index into `buffer' for scanning.  */

  memset (buffer, 0, sizeof (buffer));
  fseek (f, 0, SEEK_END);
  pos = ftell (f);

  /* Set `bytes_read' to the size of the last, probably partial, buffer; 0 <
   * `bytes_read' <= `BUFSIZ'.  */
  bytes_read = pos % BUFSIZ;
  if (bytes_read == 0)
    bytes_read = BUFSIZ;
  /* Make `pos' a multiple of `BUFSIZ' (0 if the file is short), so that all
   * reads will be on block boundaries, which might increase efficiency.  */
  while ((pos -= bytes_read) >= 0) {
    /* we save in the buffer at the end the first 7 chars from the last read */
    strncpy (buffer + BUFSIZ, buffer, 5 + 2);   /* 2 == 2 * str_len(CRLF) */
    fseek (f, pos, SEEK_SET);
    bytes_read = fread (buffer, sizeof (char), bytes_read, f);
    if (bytes_read == -1)
      return -1;
    for (i = bytes_read; --i >= 0;)
      if (!str_ncmp (buffer + i, "\n\nFrom ", str_len ("\n\nFrom "))) { /* found it - go to the beginning of the From */
        fseek (f, pos + i + 2, SEEK_SET);
        return 0;
      }
    bytes_read = BUFSIZ;
  }

  /* here we are at the beginning of the file */
  if (!str_ncmp ("From ", buffer, 5)) {
    fseek (f, 0, 0);
    return (0);
  }

  return (-1);
}

/* Return 1 if the last message is new */
static int test_last_status_new (FILE * f)
{
  HEADER *hdr;
  ENVELOPE *tmp_envelope;
  int result = 0;

  if (fseek_last_message (f) == -1)
    return (0);

  hdr = mutt_new_header ();
  tmp_envelope = mutt_read_rfc822_header (f, hdr, 0, 0);
  if (!(hdr->read || hdr->old))
    result = 1;

  mutt_free_envelope (&tmp_envelope);
  mutt_free_header (&hdr);

  return result;
}

static int test_new_folder (const char *path)
{
  FILE *f;
  int rc = 0;
  int typ;

  typ = mx_get_magic (path);

  if (typ != M_MBOX && typ != M_MMDF)
    return 0;

  if ((f = fopen (path, "rb"))) {
    rc = test_last_status_new (f);
    fclose (f);
  }

  return rc;
}

BUFFY *buffy_find_mailbox (const char *path)
{
  struct stat sb;
  struct stat tmp_sb;
  int i = 0;

  if (stat (path, &sb) != 0)
    return NULL;

  if (!list_empty(Incoming)) {
    for (i = 0; i < Incoming->length; i++) {
      if (stat (Incoming->data[i], &tmp_sb) == 0 &&
          sb.st_dev == tmp_sb.st_dev && sb.st_ino == tmp_sb.st_ino)
        return ((BUFFY*) Incoming->data[i]);
    }
  }
  return (NULL);
}

void buffy_update_mailbox (BUFFY * b)
{
  struct stat sb;

  if (!b)
    return;

  if (stat (b->path, &sb) == 0)
    b->size = (long) sb.st_size;
  else
    b->size = 0;
  return;
}
#endif

/* func to free buffy for list_del() */
static void buffy_free (BUFFY** p) {
  FREE(&(*p)->path);
  FREE(p);
}

int buffy_lookup (const char* path) {
  int i = 0;
  if (list_empty(Incoming) || !path || !*path)
    return (-1);
  for (i = 0; i < Incoming->length; i++) {
    if (str_eq (((BUFFY*) Incoming->data[i])->path, path) )
      return (i);
  }
  return (-1);
}

int buffy_parse_mailboxes (BUFFER * path, BUFFER * s, unsigned long data,
                          BUFFER * err)
{
  BUFFY* tmp;
  char buf[_POSIX_PATH_MAX];
  int i = 0;
#ifdef BUFFY_SIZE
  struct stat sb;
#endif /* BUFFY_SIZE */

  while (MoreArgs (s)) {
    mutt_extract_token (path, s, 0);
    strfcpy (buf, path->data, sizeof (buf));

    if (data == M_UNMAILBOXES && str_eq (buf, "*") == 0) {
      list_del (&Incoming, (list_del_t*) buffy_free);
      return 0;
    }

    /* Skip empty tokens. */
    if (!*buf)
      continue;

    mutt_expand_path (buf, sizeof (buf));
    i = buffy_lookup (buf);

    if (data == M_UNMAILBOXES) {
      if (i >= 0) {
        tmp = (BUFFY*) list_pop_idx (Incoming, i);
        buffy_free (&tmp);
      }
      continue;
    }

    if (i < 0) {
      tmp = safe_calloc (1, sizeof (BUFFY));
      tmp->path = str_dup (buf);
      tmp->magic = 0;
      list_push_back (&Incoming, tmp);
      i = Incoming->length-1;
    } else
      tmp = (BUFFY*) Incoming->data[i];

    tmp->new = 0;
    tmp->notified = 1;
    tmp->newly_created = 0;

#ifdef BUFFY_SIZE
    /* for buffy_size, it is important that if the folder is new (tested by
     * reading it), the size is set to 0 so that later when we check we see
     * that it increased .  without buffy_size we probably don't care.
     */
    if (stat (tmp->path, &sb) == 0 && !test_new_folder (tmp->path)) {
      /* some systems out there don't have an off_t type */
      tmp->size = (long) sb.st_size;
    }
    else
      tmp->size = 0;
#endif /* BUFFY_SIZE */
  }
  return 0;
}

#ifdef BUFFY_SIZE
/* people use buffy_size on systems where modified time attributes are BADLY
 * broken. Ignore them.
 */
#define STAT_CHECK (sb.st_size > tmp->size)
#else
#define STAT_CHECK (sb.st_mtime > sb.st_atime || (tmp->newly_created && sb.st_ctime == sb.st_mtime && sb.st_ctime == sb.st_atime))
#endif /* BUFFY_SIZE */

/* values for force:
 * 0    don't force any checks + update sidebar
 * 1    force all checks + update sidebar
 * 2    force all checks + _don't_ update sidebar
 */
int buffy_check (int force)
{
  BUFFY *tmp;
  struct stat sb;
  struct dirent *de;
  DIR *dirp;
  char path[_POSIX_PATH_MAX];
  struct stat contex_sb;
  time_t now, last1;
  CONTEXT *ctx;
  int i = 0, local = 0, count = 0;
#ifdef USE_IMAP
  time_t last2;

  /* update postponed count as well, on force */
  if (force != 0)
    mutt_update_num_postponed ();
#endif

  /* fastest return if there are no mailboxes */
  if (list_empty(Incoming))
    return 0;
  now = time (NULL);
  if (force == 0 && (now - BuffyTime < BuffyTimeout)
#ifdef USE_IMAP
      && (now - ImapBuffyTime < ImapBuffyTimeout))
#else
    )
#endif
    return BuffyCount;

  last1 = BuffyTime;
  if (force != 0 || now - BuffyTime >= BuffyTimeout)
    BuffyTime = now;
#ifdef USE_IMAP
  last2 = ImapBuffyTime;
  if (force != 0 || now - ImapBuffyTime >= ImapBuffyTimeout)
    ImapBuffyTime = now;
#endif
  BuffyCount = 0;
  BuffyNotify = 0;

  count = sidebar_need_count ();

  if (!Context || !Context->path || 
      (mx_is_local (Context->magic-1) && stat (Context->path, &contex_sb) != 0)) {
    /* check device ID and serial number instead of comparing paths */
    contex_sb.st_dev = 0;
    contex_sb.st_ino = 0;
  }

  for (i = 0; i < Incoming->length; i++) {
    tmp = (BUFFY*) Incoming->data[i];
    tmp->magic = mx_get_magic (tmp->path);
    local = mx_is_local (tmp->magic-1);
    if ((tmp->magic <= 0 || local) && (stat (tmp->path, &sb) != 0 || sb.st_size == 0)) {
      /* if the mailbox still doesn't exist, set the newly created flag to
       * be ready for when it does. */
      tmp->newly_created = 1;
      tmp->magic = -1;
#ifdef BUFFY_SIZE
      tmp->size = 0;
#endif
      continue;
    }

    /* check to see if the folder is the currently selected folder
     * before polling */
    if (!Context || !Context->path || (local ? (sb.st_dev != contex_sb.st_dev ||
                                                sb.st_ino != contex_sb.st_ino) : 
                                       !str_eq (tmp->path, Context->path))) {
      switch (tmp->magic) {
      case M_MBOX:
      case M_MMDF:
        /* only check on force or $mail_check reached */
        if (force != 0 || (now - last1 >= BuffyTimeout)) {
          if (!count) {
            if (STAT_CHECK) {
              BuffyCount++;
              tmp->new = 1;
            }
#ifdef BUFFY_SIZE
            else {
              /* some other program has deleted mail from the folder */
              tmp->size = (long) sb.st_size;
            }
#endif
          }
          else if (STAT_CHECK || tmp->msgcount == 0) {
            /* sidebar visible */
            BuffyCount++;
            if ((ctx =
                 mx_open_mailbox (tmp->path, M_READONLY | M_QUIET | M_NOSORT | M_COUNT,
                                  NULL)) != NULL) {
              tmp->msgcount = ctx->msgcount;
              tmp->new = ctx->new;
              tmp->msg_unread = ctx->new;       /* for sidebar, wtf? */
              tmp->msg_flagged = ctx->flagged;
              mx_close_mailbox (ctx, 0);
            }
          }
          if (tmp->newly_created &&
              (sb.st_ctime != sb.st_mtime || sb.st_ctime != sb.st_atime))
            tmp->newly_created = 0;
        }
        else if (tmp->new > 0)
          BuffyCount++;
        break;

      case M_MAILDIR:
        /* only check on force or $mail_check reached */
        if (force != 0 || (now - last1 >= BuffyTimeout)) {
          snprintf (path, sizeof (path), "%s/new", tmp->path);
          if ((dirp = opendir (path)) == NULL) {
            tmp->magic = 0;
            break;
          }
          tmp->new = 0;
          tmp->msg_unread = 0;
          tmp->msgcount = 0;
          while ((de = readdir (dirp)) != NULL) {
            char *p;

            if (*de->d_name != '.' &&
                (!(p = strstr (de->d_name, ":2,")) || !strchr (p + 3, 'T'))) {
              /* one new and undeleted message is enough */
              if (tmp->new == 0) {
                BuffyCount++;
                tmp->new = 1;
                if (!count)
                  /* if sidebar invisible -> done */
                  break;
              }
              tmp->msgcount++;
              tmp->msg_unread++;
              tmp->new++;
            }
          }
          closedir (dirp);

          if (count) {
            /* only count total mail if sidebar visible */
            snprintf (path, sizeof (path), "%s/cur", tmp->path);
            if ((dirp = opendir (path)) == NULL) {
              tmp->magic = 0;
              break;
            }
            tmp->msg_flagged = 0;
            while ((de = readdir (dirp)) != NULL) {
              char *p;

              if (*de->d_name != '.'
                  && (p = strstr (de->d_name, ":2,")) != NULL) {
                if (!strchr (p + 3, 'T'))
                  tmp->msgcount++;
                if (strchr (p + 3, 'F'))
                  tmp->msg_flagged++;
              }
            }
            closedir (dirp);
          }
        }
        else if (tmp->new > 0)
          /* keep current stats if !force and !$mail_check reached */
          BuffyCount++;
        break;

      case M_MH:
        /* only check on force or $mail_check reached */
        if (force != 0 || (now - last1 >= BuffyTimeout)) {
          if ((tmp->new = mh_buffy (tmp->path)) > 0)
            BuffyCount++;
          if (count) {
            DIR *dp;
            struct dirent *de;

            if ((dp = opendir (path)) == NULL)
              break;
            tmp->new = 0;
            tmp->msgcount = 0;
            tmp->msg_unread = 0;
            while ((de = readdir (dp))) {
              if (mh_valid_message (de->d_name)) {
                tmp->msgcount++;
                tmp->msg_unread++;
                tmp->new++;
              }
            }
            closedir (dp);
          }
        }
        else if (tmp->new > 0)
          /* keep current stats if !force and !$mail_check reached */
          BuffyCount++;
        break;

#ifdef USE_IMAP
      case M_IMAP:
        /* only check on force or $imap_mail_check reached */
        if (force != 0 || (now - last2 >= ImapBuffyTimeout)) {
          tmp->msgcount = imap_mailbox_check (tmp->path, 0);
          if ((tmp->new = imap_mailbox_check (tmp->path, 1)) > 0) {
            BuffyCount++;
            tmp->msg_unread = tmp->new; /* for sidebar; wtf? */
          }
          else {
            tmp->new = 0;
            tmp->msg_unread = 0;
          }
        }
        else if (tmp->new > 0)
          /* keep current stats if !force and !$imap_mail_check reached */
          BuffyCount++;
        break;
#endif

      }
    }
#ifdef BUFFY_SIZE
    else if (Context && Context->path)
      tmp->size = (long) sb.st_size;    /* update the size */
#endif

    if (tmp->new <= 0)
      tmp->notified = 0;
    else if (!tmp->notified)
      BuffyNotify++;
    tmp->has_new = tmp->new > 0;
  }
  if (BuffyCount > 0 && force != 2)
    sidebar_draw (CurrentMenu);
  return (BuffyCount);
}

int buffy_list (void)
{
  BUFFY *tmp;
  char path[_POSIX_PATH_MAX];
  char buffylist[160];
  int pos;
  int first;
  int have_unnotified = BuffyNotify;
  int i = 0;

  if (option (OPTFORCEBUFFYCHECK))
    buffy_check (1);

  pos = 0;
  first = 1;
  buffylist[0] = 0;
  pos += str_len (strncat (buffylist, _("New mail in "), sizeof (buffylist) - 1 - pos)); /* __STRNCAT_CHECKED__ */
  if (Incoming) {
    for (i = 0; i < Incoming->length; i++) {
      tmp = (BUFFY*) Incoming->data[i];
      /* Is there new mail in this mailbox? */
      if (tmp->new <= 0 || (have_unnotified && tmp->notified))
        continue;

      strfcpy (path, tmp->path, sizeof (path));
      mutt_pretty_mailbox (path);

      if (!first && pos + str_len (path) >= COLS - 7)
        break;

      if (!first)
        pos += str_len (strncat (buffylist + pos, ", ", sizeof (buffylist) - 1 - pos));    /* __STRNCAT_CHECKED__ */

      /* Prepend an asterisk to mailboxes not already notified */
      if (!tmp->notified) {
        /* pos += str_len (strncat(buffylist + pos, "*", sizeof(buffylist)-1-pos));  __STRNCAT_CHECKED__ */
        tmp->notified = 1;
        BuffyNotify--;
      }
      pos += str_len (strncat (buffylist + pos, path, sizeof (buffylist) - 1 - pos));      /* __STRNCAT_CHECKED__ */
      first = 0;
    }
  }
  if (!first && i < Incoming->length) {
    strncat (buffylist + pos, ", ...", sizeof (buffylist) - 1 - pos);   /* __STRNCAT_CHECKED__ */
  }
  if (!first) {
    /* on new mail: redraw sidebar */
    sidebar_draw (CurrentMenu);
    mutt_message ("%s", buffylist);
    return (1);
  }
  /* there were no mailboxes needing to be notified, so clean up since 
   * BuffyNotify has somehow gotten out of sync
   */
  BuffyNotify = 0;
  return (0);
}

int buffy_notify (void)
{
  if (buffy_check (0) && BuffyNotify) {
    return (buffy_list ());
  }
  return (0);
}

/* 
 * mutt_buffy() -- incoming folders completion routine
 *
 * given a folder name, this routine gives the next incoming folder with new
 * new mail.
 */
void buffy_next (char *s, size_t slen)
{
  int i = 0, c = 0, l = 0;

  if (list_empty(Incoming))
    return;

  mutt_expand_path (s, _POSIX_PATH_MAX);
  if (buffy_check (0) == 0) {
    *s = '\0';
    return;
  }

  /*
   * If buffy_lookup returns the index,
   * or -1 if not found (-1..Incoming->length-1);
   * plus one --> (0..Incoming->length).
   * Modulo mapps it into the correct range.
   */
  i = 1 + buffy_lookup (s);
  for (l=0; l < Incoming->length; l++) {
    c = (l+i) % Incoming->length;
    if ((!Context || !Context->path || !str_eq (((BUFFY*) Incoming->data[c])->path, Context->path)) &&
        ((BUFFY*) Incoming->data[c])->new > 0)
      break;
  }
  if (l >= Incoming->length) {
    *s = '\0';
    /* something went wrong since we're here when buffy_check
     * reported new mail */
    buffy_check (0);
  } else {
    strfcpy (s, ((BUFFY*) Incoming->data[c])->path, slen);
    mutt_pretty_mailbox (s);
  }
}
