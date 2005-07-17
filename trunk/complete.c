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
#ifdef USE_IMAP
#include "mx.h"
#include "imap.h"
#endif
#ifdef USE_NNTP
#include "nntp.h"
#endif

#include "lib/str.h"
#include "lib/debug.h"

#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

/* given a partial pathname, this routine fills in as much of the rest of the
 * path as is unique.
 *
 * return 0 if ok, -1 if no matches
 */
int mutt_complete (char *s, size_t slen)
{
  char *p;
  DIR *dirp = NULL;
  struct dirent *de;
  int i, init = 0;
  size_t len;
  char dirpart[_POSIX_PATH_MAX], exp_dirpart[_POSIX_PATH_MAX];
  char filepart[_POSIX_PATH_MAX];

#ifdef USE_IMAP
  char imap_path[LONG_STRING];
#endif

  debug_print (2, ("completing %s\n", s));

#ifdef USE_NNTP
  if (option (OPTNEWS)) {
    LIST *l = CurrentNewsSrv->list;

    strfcpy (filepart, s, sizeof (filepart));

    /*
     * special case to handle when there is no filepart yet.
     * find the first subscribed newsgroup
     */
    if ((len = mutt_strlen (filepart)) == 0) {
      for (; l; l = l->next) {
        NNTP_DATA *data = (NNTP_DATA *) l->data;

        if (data && data->subscribed) {
          strfcpy (filepart, data->group, sizeof (filepart));
          init++;
          l = l->next;
          break;
        }
      }
    }

    for (; l; l = l->next) {
      NNTP_DATA *data = (NNTP_DATA *) l->data;

      if (data && data->subscribed &&
          safe_strncmp (data->group, filepart, len) == 0) {
        if (init) {
          for (i = 0; filepart[i] && data->group[i]; i++) {
            if (filepart[i] != data->group[i]) {
              filepart[i] = 0;
              break;
            }
          }
          filepart[i] = 0;
        }
        else {
          strfcpy (filepart, data->group, sizeof (filepart));
          init = 1;
        }
      }
    }

    strcpy (s, filepart);

    return (init ? 0 : -1);
  }
#endif

#ifdef USE_IMAP
  /* we can use '/' as a delimiter, imap_complete rewrites it */
  if (*s == '=' || *s == '+' || *s == '!') {
    if (*s == '!')
      p = NONULL (Spoolfile);
    else
      p = NONULL (Maildir);

    mutt_concat_path (imap_path, p, s + 1, sizeof (imap_path));
  }
  else
    strfcpy (imap_path, s, sizeof (imap_path));

  if (mx_get_magic (imap_path) == M_IMAP)
    return imap_complete (s, slen, imap_path);
#endif

  if (*s == '=' || *s == '+' || *s == '!') {
    dirpart[0] = *s;
    dirpart[1] = 0;
    if (*s == '!')
      strfcpy (exp_dirpart, NONULL (Spoolfile), sizeof (exp_dirpart));
    else
      strfcpy (exp_dirpart, NONULL (Maildir), sizeof (exp_dirpart));
    if ((p = strrchr (s, '/'))) {
      char buf[_POSIX_PATH_MAX];

      *p++ = 0;
      mutt_concat_path (buf, exp_dirpart, s + 1, sizeof (buf));
      strfcpy (exp_dirpart, buf, sizeof (exp_dirpart));
      snprintf (buf, sizeof (buf), "%s%s/", dirpart, s + 1);
      strfcpy (dirpart, buf, sizeof (dirpart));
      strfcpy (filepart, p, sizeof (filepart));
    }
    else
      strfcpy (filepart, s + 1, sizeof (filepart));
    dirp = opendir (exp_dirpart);
  }
  else {
    if ((p = strrchr (s, '/'))) {
      if (p == s) {             /* absolute path */
        p = s + 1;
        strfcpy (dirpart, "/", sizeof (dirpart));
        exp_dirpart[0] = 0;
        strfcpy (filepart, p, sizeof (filepart));
        dirp = opendir (dirpart);
      }
      else {
        *p = 0;
        len = (size_t) (p - s);
        strncpy (dirpart, s, len);
        dirpart[len] = 0;
        p++;
        strfcpy (filepart, p, sizeof (filepart));
        strfcpy (exp_dirpart, dirpart, sizeof (exp_dirpart));
        mutt_expand_path (exp_dirpart, sizeof (exp_dirpart));
        dirp = opendir (exp_dirpart);
      }
    }
    else {
      /* no directory name, so assume current directory. */
      dirpart[0] = 0;
      strfcpy (filepart, s, sizeof (filepart));
      dirp = opendir (".");
    }
  }

  if (dirp == NULL) {
    debug_print (1, ("%s: %s (errno %d).\n", exp_dirpart, strerror (errno), errno));
    return (-1);
  }

  /*
   * special case to handle when there is no filepart yet.  find the first
   * file/directory which is not ``.'' or ``..''
   */
  if ((len = mutt_strlen (filepart)) == 0) {
    while ((de = readdir (dirp)) != NULL) {
      if (mutt_strcmp (".", de->d_name) != 0
          && mutt_strcmp ("..", de->d_name) != 0) {
        strfcpy (filepart, de->d_name, sizeof (filepart));
        init++;
        break;
      }
    }
  }

  while ((de = readdir (dirp)) != NULL) {
    if (safe_strncmp (de->d_name, filepart, len) == 0) {
      if (init) {
        for (i = 0; filepart[i] && de->d_name[i]; i++) {
          if (filepart[i] != de->d_name[i]) {
            filepart[i] = 0;
            break;
          }
        }
        filepart[i] = 0;
      }
      else {
        char buf[_POSIX_PATH_MAX];
        struct stat st;

        strfcpy (filepart, de->d_name, sizeof (filepart));

        /* check to see if it is a directory */
        if (dirpart[0]) {
          strfcpy (buf, exp_dirpart, sizeof (buf));
          strfcpy (buf + mutt_strlen (buf), "/", sizeof (buf) - mutt_strlen (buf));
        }
        else
          buf[0] = 0;
        strfcpy (buf + mutt_strlen (buf), filepart, sizeof (buf) - mutt_strlen (buf));
        if (stat (buf, &st) != -1 && (st.st_mode & S_IFDIR))
          strfcpy (filepart + mutt_strlen (filepart), "/",
                   sizeof (filepart) - mutt_strlen (filepart));
        init = 1;
      }
    }
  }
  closedir (dirp);

  if (dirpart[0]) {
    strfcpy (s, dirpart, slen);
    if (mutt_strcmp ("/", dirpart) != 0 && dirpart[0] != '='
        && dirpart[0] != '+')
      strfcpy (s + mutt_strlen (s), "/", slen - mutt_strlen (s));
    strfcpy (s + mutt_strlen (s), filepart, slen - mutt_strlen (s));
  }
  else
    strfcpy (s, filepart, slen);

  return (init ? 0 : -1);
}
