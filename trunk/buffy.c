/* 
 * Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 * 
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 * 
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 * 
 *     You should have received a copy of the GNU General Public License
 *     along with this program; if not, write to the Free Software
 *     Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.
 */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "mutt.h"
#include "buffy.h"
#include "mailbox.h"
#include "mx.h"
#include "sidebar.h"

#include "mutt_curses.h"

#ifdef USE_IMAP
#include "imap.h"
#endif

#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <utime.h>
#include <ctype.h>
#include <unistd.h>

#include <stdio.h>

static time_t BuffyTime = 0;            /* last time we started checking for mail */
#ifdef USE_IMAP
static time_t ImapBuffyTime = 0;        /* last time we started checking for mail */
#endif
static short BuffyCount = 0;            /* how many boxes with new mail */
static short BuffyNotify = 0;           /* # of unnotified new boxes */

#ifdef BUFFY_SIZE

/* Find the last message in the file. 
 * upon success return 0. If no message found - return -1 */

int fseek_last_message (FILE * f)
{
  long int pos;
  char buffer[BUFSIZ + 9];	/* 7 for "\n\nFrom " */
  int bytes_read;
  int i;			/* Index into `buffer' for scanning.  */

  memset (buffer, 0, sizeof(buffer));
  fseek (f, 0, SEEK_END);
  pos = ftell (f);

  /* Set `bytes_read' to the size of the last, probably partial, buffer; 0 <
   * `bytes_read' <= `BUFSIZ'.  */
  bytes_read = pos % BUFSIZ;
  if (bytes_read == 0)
    bytes_read = BUFSIZ;
  /* Make `pos' a multiple of `BUFSIZ' (0 if the file is short), so that all
   * reads will be on block boundaries, which might increase efficiency.  */
  while ((pos -= bytes_read) >= 0)
  {
    /* we save in the buffer at the end the first 7 chars from the last read */
    strncpy (buffer + BUFSIZ, buffer, 5+2); /* 2 == 2 * mutt_strlen(CRLF) */
    fseek (f, pos, SEEK_SET);
    bytes_read = fread (buffer, sizeof (char), bytes_read, f);
    if (bytes_read == -1)
      return -1;
    for (i = bytes_read; --i >= 0;)
      if (!mutt_strncmp (buffer + i, "\n\nFrom ", mutt_strlen ("\n\nFrom ")))
      {				/* found it - go to the beginning of the From */
	fseek (f, pos + i + 2, SEEK_SET);
	return 0;
      }
    bytes_read = BUFSIZ;
  }

  /* here we are at the beginning of the file */
  if (!mutt_strncmp ("From ", buffer, 5))
  {
    fseek (f, 0, 0);
    return (0);
  }

  return (-1);
}

/* Return 1 if the last message is new */
int test_last_status_new (FILE * f)
{
  HEADER *hdr;
  ENVELOPE* tmp_envelope;
  int result = 0;

  if (fseek_last_message (f) == -1)
    return (0);

  hdr = mutt_new_header ();
  tmp_envelope = mutt_read_rfc822_header (f, hdr, 0, 0);
  if (!(hdr->read || hdr->old))
    result = 1;

  mutt_free_envelope(&tmp_envelope);
  mutt_free_header (&hdr);

  return result;
}

int test_new_folder (const char *path)
{
  FILE *f;
  int rc = 0;
  int typ;

  typ = mx_get_magic (path);

  if (typ != M_MBOX && typ != M_MMDF)
    return 0;

  if ((f = fopen (path, "rb")))
  {
    rc = test_last_status_new (f);
    fclose (f);
  }

  return rc;
}

BUFFY *mutt_find_mailbox (const char *path)
{
  BUFFY *tmp = NULL;
  struct stat sb;
  struct stat tmp_sb;
  
  if (stat (path,&sb) != 0)
    return NULL;

  for (tmp = Incoming; tmp; tmp = tmp->next)
  {
    if (stat (tmp->path,&tmp_sb) ==0 && 
	sb.st_dev == tmp_sb.st_dev && sb.st_ino == tmp_sb.st_ino)
      break;
  }
  return tmp;
}

void mutt_update_mailbox (BUFFY * b)
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

int mutt_parse_mailboxes (BUFFER *path, BUFFER *s, unsigned long data, BUFFER *err)
{
  BUFFY **tmp,*tmp1,*last;
  char buf[_POSIX_PATH_MAX];
  int dup = 0;
#ifdef BUFFY_SIZE
  struct stat sb;
#endif /* BUFFY_SIZE */

  /*
   * FIXME
   * to get rid of correcting the ->prev pointers in sidebar.c,
   * correct them right here
   */

  /*
   * FIXME
   * if we really want to make the sort order of the sidebar
   * configurable, this has to go right here
   */
  
  while (MoreArgs (s))
  {
    mutt_extract_token (path, s, 0);
    strfcpy (buf, path->data, sizeof (buf));

    if(data == M_UNMAILBOXES && mutt_strcmp(buf,"*") == 0)
    {
      for (tmp = &Incoming; *tmp;)
      {
        FREE (&((*tmp)->path));
        tmp1=(*tmp)->next;
        FREE (tmp);
        *tmp=tmp1;
      }
      return 0;
    }

    mutt_expand_path (buf, sizeof (buf));

    /* Skip empty tokens. */
    if(!*buf) continue;

    /* simple check to avoid duplicates */
    dup = 0;
    for (tmp = &Incoming; *tmp && dup == 0; tmp = &((*tmp)->next))
    {
      if (mutt_strcmp (buf, (*tmp)->path) == 0) {
        dup = 1;
        break;
      }
    }

    if (dup == 1)
      continue;
    tmp = &Incoming;

    if(data == M_UNMAILBOXES)
    {
      if(*tmp)
      {
        FREE (&((*tmp)->path));
        tmp1=(*tmp)->next;
        FREE (tmp);
        *tmp=tmp1;
      }
      continue;
    }

    /* loop over list while it's sorted */
    tmp1 = NULL;
    last = NULL;
    for (tmp = &Incoming; *tmp ; tmp = &((*tmp)->next)) {
      /*
       * FIXME
       * change this to get whatever sorting order
       */
      if (mutt_strcmp (buf, (*tmp)->path) < 0) {
        tmp1 = (*tmp);
        break;
      }
      last = (*tmp);
    }

    /* we want: last -> tmp -> tmp1 */
    *tmp = (BUFFY *) safe_calloc (1, sizeof (BUFFY));
    (*tmp)->path = safe_strdup (buf);
    (*tmp)->magic = 0;

    /* correct pointers */
    (*tmp)->next = tmp1;
    if (last)
      last->next = (*tmp);

    /* left as-is */
    (*tmp)->new = 0;
    (*tmp)->notified = 1;
    (*tmp)->newly_created = 0;

#ifdef BUFFY_SIZE
    /* for buffy_size, it is important that if the folder is new (tested by
     * reading it), the size is set to 0 so that later when we check we see
     * that it increased .  without buffy_size we probably don't care.
     */
    if (stat ((*tmp)->path, &sb) == 0 && !test_new_folder ((*tmp)->path))
    {
      /* some systems out there don't have an off_t type */
      (*tmp)->size = (long) sb.st_size;
    }
    else
      (*tmp)->size = 0;
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

int mutt_buffy_check (int force)
{
  BUFFY *tmp;
  struct stat sb;
  struct dirent *de;
  DIR *dirp;
  char path[_POSIX_PATH_MAX];
  struct stat contex_sb;
  time_t now, last1, last2;
  CONTEXT *ctx;
#ifdef USE_IMAP
  /* update postponed count as well, on force */
  if (force)
    mutt_update_num_postponed ();
#endif

  /* fastest return if there are no mailboxes */
  if (!Incoming)
    return 0;
  now = time (NULL);
  if (!force && (now - BuffyTime < BuffyTimeout)
#ifdef USE_IMAP
      && (now - ImapBuffyTime < ImapBuffyTimeout))
#else
      )
#endif
    return BuffyCount;

  last1 = BuffyTime;
  if (force || now - BuffyTime >= BuffyTimeout)
    BuffyTime = now;
#ifdef USE_IMAP
  last2 = ImapBuffyTime;
  if (force || now - ImapBuffyTime >= ImapBuffyTimeout)
    ImapBuffyTime = now;
#endif
  BuffyCount = 0;
  BuffyNotify = 0;

#ifdef USE_IMAP
  if (!Context || Context->magic != M_IMAP)
#endif
#ifdef USE_POP
  if (!Context || Context->magic != M_POP)
#endif
#ifdef USE_NNTP
  if (!Context || Context->magic != M_NNTP)
#endif
  /* check device ID and serial number instead of comparing paths */
  if (!Context || !Context->path || stat (Context->path, &contex_sb) != 0)
  {
    contex_sb.st_dev=0;
    contex_sb.st_ino=0;
  }
  
  for (tmp = Incoming; tmp; tmp = tmp->next)
  {
#ifdef USE_IMAP
    if (mx_is_imap (tmp->path))
      tmp->magic = M_IMAP;
    else
#endif
#ifdef USE_POP
    if (mx_is_pop (tmp->path))
      tmp->magic = M_POP;
    else
#endif
#ifdef USE_NNTP
    if ((tmp->magic == M_NNTP) || mx_is_nntp (tmp->path))
      tmp->magic = M_NNTP;
    else
#endif
    if (stat (tmp->path, &sb) != 0 || sb.st_size == 0 ||
       (!tmp->magic && (tmp->magic = mx_get_magic (tmp->path)) <= 0))
    {
      /* if the mailbox still doesn't exist, set the newly created flag to
       * be ready for when it does. */
      tmp->newly_created = 1;
      tmp->magic = 0;
#ifdef BUFFY_SIZE
      tmp->size = 0;
#endif
      continue;
    }

    /* check to see if the folder is the currently selected folder
     * before polling */
    if (!Context || !Context->path ||
         (
           (0
#ifdef USE_IMAP
            || tmp->magic == M_IMAP
#endif
#ifdef USE_POP
            || tmp->magic == M_POP
#endif
#ifdef USE_NNTP
            || tmp->magic == M_NNTP
#endif
           ) ? mutt_strcmp (tmp->path, Context->path) :
               (sb.st_dev != contex_sb.st_dev || sb.st_ino != contex_sb.st_ino)
         )
       )
    {
      switch (tmp->magic)
      {
      case M_MBOX:
      case M_MMDF:
        /* only check on force or $mail_check reached */
        if (force || (now - last1 >= BuffyTimeout)) {
          tmp->new = 0;
          tmp->msg_unread = 0;
          if (SidebarWidth == 0 || !option (OPTMBOXPANE)) {
            if (STAT_CHECK) {
              BuffyCount++;
              tmp->new = tmp->has_new = 1;
            }
#ifdef BUFFY_SIZE
            else
            {
              /* some other program has deleted mail from the folder */
              tmp->size = (long) sb.st_size;
            }
#endif
          } else if (SidebarWidth > 0 && option (OPTMBOXPANE) && 
                    (STAT_CHECK || tmp->msgcount == 0)) {
            /* sidebar visible */
            int msg_count = 0, msg_new = 0, msg_unread = 0;
            BuffyCount++;
            if ((ctx = mx_open_mailbox (tmp->path, M_READONLY | M_QUIET | M_NOSORT, NULL)) != NULL) {
              msg_count = ctx->msgcount;
              msg_new = ctx->new;
              msg_unread = ctx->unread;
              mx_close_mailbox (ctx, 0);
            }
            tmp->msgcount = msg_count;
            tmp->new = msg_new;
            tmp->msg_unread = msg_unread;
            tmp->has_new = msg_new > 0;
          }
          if (tmp->newly_created &&
              (sb.st_ctime != sb.st_mtime || sb.st_ctime != sb.st_atime))
            tmp->newly_created = 0;
          tmp->has_new = tmp->new > 0;
        } else if (tmp->new > 0) {
          /* keep current stats if !force and !$mail_check reached */
          BuffyCount++;
          tmp->has_new = 1;
        }
        break;

      case M_MAILDIR:
        /* only check on force or $mail_check reached */
        if (force || (now - last1 >= BuffyTimeout)) {
          snprintf (path, sizeof (path), "%s/new", tmp->path);
          if ((dirp = opendir (path)) == NULL)
          {
            tmp->magic = 0;
            break;
          }
          tmp->new = 0;
          tmp->msg_unread = 0;
          tmp->msgcount = 0;
          while ((de = readdir (dirp)) != NULL)
          {
            char *p;
            if (*de->d_name != '.' && 
                (!(p = strstr (de->d_name, ":2,")) || !strchr (p + 3, 'T')))
            {
              /* one new and undeleted message is enough */
              if (tmp->new == 0)
              {
                BuffyCount++;
                tmp->has_new = tmp->new = 1;
                if (SidebarWidth == 0 || !option (OPTMBOXPANE))
                  /* if sidebar invisible -> done */
                  break;
              }
              tmp->msgcount++;
              tmp->msg_unread++;
              tmp->new++;
            }
          }
          closedir (dirp);
          tmp->has_new = tmp->new > 0;

          if (SidebarWidth > 0 && option (OPTMBOXPANE))
          {
            /* only count total mail if sidebar visible */
            snprintf (path, sizeof (path), "%s/cur", tmp->path);
            if ((dirp = opendir (path)) == NULL)
            {
              tmp->magic = 0;
              break;
            }
            while ((de = readdir (dirp)) != NULL)
            {
              char *p;
              if (*de->d_name != '.' && 
                  (!(p = strstr (de->d_name, ":2,")) || !strchr (p + 3, 'T')))
              {
                tmp->msgcount++;
              }
            }
            closedir (dirp);
          }
        } else if (tmp->new > 0) {
          /* keep current stats if !force and !$mail_check reached */
          BuffyCount++;
          tmp->has_new = 1;
        }
        break;

      case M_MH:
        /* only check on force or $mail_check reached */
        if (force || (now - last1 >= BuffyTimeout)) {
          tmp->new = 0;
          tmp->msg_unread = 0;
          if ((tmp->new = mh_buffy (tmp->path)) > 0)
            BuffyCount++;
          if (SidebarWidth > 0 && option (OPTMBOXPANE))
          {
            DIR *dp;
            struct dirent *de;
            if ((dp = opendir (path)) == NULL)
              break;
                tmp->msgcount = 0;
            while ((de = readdir (dp)))
            {
              if (mh_valid_message (de->d_name))
              {
                tmp->msgcount++;
                tmp->msg_unread++;
                tmp->new++;
              }
            }
            closedir (dp);
          }
          tmp->has_new = tmp->new > 0;
        } else if (tmp->new > 0) {
          /* keep current stats if !force and !$mail_check reached */
          BuffyCount++;
          tmp->has_new = 1;
        }
        break;

#ifdef USE_IMAP
      case M_IMAP:
        /* only check on force or $imap_mail_check reached */
        if (force || (now - last2 >= ImapBuffyTimeout)) {
          tmp->new = 0;
          tmp->msg_unread = 0;
          tmp->msgcount = imap_mailbox_check (tmp->path, 0);
          if ((tmp->new = imap_mailbox_check (tmp->path, 1)) > 0) {
            BuffyCount++;
            tmp->has_new = tmp->new > 0;
            tmp->msg_unread = tmp->new; /* for sidebar; wtf? */
          }
          else
            tmp->new = 0;
        } else if (tmp->new > 0) {
          /* keep current stats if !force and !$imap_mail_check reached */
          BuffyCount++;
          tmp->has_new = 1;
        }
        break;
#endif

#ifdef USE_POP
      case M_POP:
        break;
#endif

#ifdef USE_NNTP
      case M_NNTP:
        break;
#endif
      }
    }
#ifdef BUFFY_SIZE
    else if (Context && Context->path)
      tmp->size = (long) sb.st_size;	/* update the size */
#endif

    if (!tmp->new)
      tmp->notified = 0;
    else if (!tmp->notified)
      BuffyNotify++;
    tmp->has_new = tmp->new > 0;
  }
  if (BuffyCount > 0)
    draw_sidebar (CurrentMenu);
  return (BuffyCount);
}

int mutt_buffy_list (void)
{
  BUFFY *tmp;
  char path[_POSIX_PATH_MAX];
  char buffylist[160];
  int pos;
  int first;

  int have_unnotified = BuffyNotify;
  
  pos = 0;
  first = 1;
  buffylist[0] = 0;
  pos += strlen (strncat (buffylist, _("New mail in "), sizeof (buffylist) - 1 - pos)); /* __STRNCAT_CHECKED__ */
  for (tmp = Incoming; tmp; tmp = tmp->next)
  {
    /* Is there new mail in this mailbox? */
    if (!tmp->new || (have_unnotified && tmp->notified))
      continue;

    strfcpy (path, tmp->path, sizeof (path));
    mutt_pretty_mailbox (path);
    
    if (!first && pos + strlen (path) >= COLS - 7)
      break;
    
    if (!first)
      pos += strlen (strncat(buffylist + pos, ", ", sizeof(buffylist)-1-pos)); /* __STRNCAT_CHECKED__ */

    /* Prepend an asterisk to mailboxes not already notified */
    if (!tmp->notified)
    {
      /* pos += strlen (strncat(buffylist + pos, "*", sizeof(buffylist)-1-pos));  __STRNCAT_CHECKED__ */
      tmp->notified = 1;
      BuffyNotify--;
    }
    pos += strlen (strncat(buffylist + pos, path, sizeof(buffylist)-1-pos)); /* __STRNCAT_CHECKED__ */
    first = 0;
  }
  if (!first && tmp)
  {
    strncat (buffylist + pos, ", ...", sizeof (buffylist) - 1 - pos); /* __STRNCAT_CHECKED__ */
  }
  if (!first)
  {
    /* on new mail: redraw sidebar */
    draw_sidebar (CurrentMenu);
    mutt_message ("%s", buffylist);
    return (1);
  }
  /* there were no mailboxes needing to be notified, so clean up since 
   * BuffyNotify has somehow gotten out of sync
   */
  BuffyNotify = 0;
  return (0);
}

int mutt_buffy_notify (void)
{
  if (mutt_buffy_check (0) && BuffyNotify)
  {
    return (mutt_buffy_list ());
  }
  return (0);
}

/* 
 * mutt_buffy() -- incoming folders completion routine
 *
 * given a folder name, this routine gives the next incoming folder with new
 * new mail.
 */
void mutt_buffy (char *s, size_t slen)
{
  int count;
  BUFFY *tmp = Incoming;

  mutt_expand_path (s, _POSIX_PATH_MAX);
  switch (mutt_buffy_check (0))
  {
  case 0:

    *s = '\0';
    break;

  case 1:

    while (tmp && !tmp->new)
      tmp = tmp->next;
    if (!tmp)
    {
      *s = '\0';
      mutt_buffy_check (1); /* buffy was wrong - resync things */
      break;
    }
    strfcpy (s, tmp->path, slen);
    mutt_pretty_mailbox (s);
    break;

  default:
    
    count = 0;
    while (count < 3)
    {
      if (mutt_strcmp (s, tmp->path) == 0)
	count++;
      else if (count && tmp->new)
	break;
      tmp = tmp->next;
      if (!tmp)
      {
	tmp = Incoming;
	count++;
      }
    }
    if (count >= 3)
    {
      *s = '\0';
      mutt_buffy_check (1); /* buffy was wrong - resync things */
      break;
    }
    strfcpy (s, tmp->path, slen);
    mutt_pretty_mailbox (s);
    break;
  }
}
