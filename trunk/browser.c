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
#include "enter.h"
#include "mx.h"
#include "mutt_curses.h"
#include "mutt_menu.h"
#include "buffy.h"
#include "mapping.h"
#include "sort.h"
#include "browser.h"
#include "attach.h"

#ifdef USE_IMAP
#include "imap.h"
#include "imap/mx_imap.h"
#endif
#ifdef USE_NNTP
#include "nntp.h"
#endif
#include "sidebar.h"

#include "lib/mem.h"
#include "lib/intl.h"
#include "lib/str.h"
#include "lib/list.h"

#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

static struct mapping_t FolderHelp[] = {
  {N_("Exit"), OP_EXIT},
  {N_("Chdir"), OP_CHANGE_DIRECTORY},
  {N_("Mask"), OP_ENTER_MASK},
  {N_("Help"), OP_HELP},
  {NULL}
};

#ifdef USE_NNTP
static struct mapping_t FolderNewsHelp[] = {
  {N_("Exit"), OP_EXIT},
  {N_("List"), OP_TOGGLE_MAILBOXES},
  {N_("Subscribe"), OP_BROWSER_SUBSCRIBE},
  {N_("Unsubscribe"), OP_BROWSER_UNSUBSCRIBE},
  {N_("Catchup"), OP_CATCHUP},
  {N_("Mask"), OP_ENTER_MASK},
  {N_("Help"), OP_HELP},
  {NULL}
};
#endif

typedef struct folder_t {
  struct folder_file *ff;
  int num;
} FOLDER;

static char LastDir[_POSIX_PATH_MAX] = "";
static char LastDirBackup[_POSIX_PATH_MAX] = "";

/* Frees up the memory allocated for the local-global variables.  */
static void destroy_state (struct browser_state *state)
{
  int c;

  for (c = 0; c < state->entrylen; c++) {
    mem_free (&((state->entry)[c].name));
    mem_free (&((state->entry)[c].desc));
    mem_free (&((state->entry)[c].st));
  }
#ifdef USE_IMAP
  mem_free (&state->folder);
#endif
  mem_free (&state->entry);
}

static int browser_compare_subject (const void *a, const void *b)
{
  struct folder_file *pa = (struct folder_file *) a;
  struct folder_file *pb = (struct folder_file *) b;

  int r = str_coll (pa->name, pb->name);

  return ((BrowserSort & SORT_REVERSE) ? -r : r);
}

static int browser_compare_date (const void *a, const void *b)
{
  struct folder_file *pa = (struct folder_file *) a;
  struct folder_file *pb = (struct folder_file *) b;

  int r = pa->mtime - pb->mtime;

  return ((BrowserSort & SORT_REVERSE) ? -r : r);
}

static int browser_compare_size (const void *a, const void *b)
{
  struct folder_file *pa = (struct folder_file *) a;
  struct folder_file *pb = (struct folder_file *) b;

  int r = pa->size - pb->size;

  return ((BrowserSort & SORT_REVERSE) ? -r : r);
}

static void browser_sort (struct browser_state *state)
{
  int (*f) (const void *, const void *);

  switch (BrowserSort & SORT_MASK) {
  case SORT_ORDER:
    return;
  case SORT_DATE:
#ifdef USE_NNTP
    if (option (OPTNEWS))
      return;
#endif
    f = browser_compare_date;
    break;
  case SORT_SIZE:
#ifdef USE_NNTP
    if (option (OPTNEWS))
      return;
#endif
    f = browser_compare_size;
    break;
  case SORT_SUBJECT:
  default:
    f = browser_compare_subject;
    break;
  }
  qsort (state->entry, state->entrylen, sizeof (struct folder_file), f);
}

static int link_is_dir (const char *folder, const char *path)
{
  struct stat st;
  char fullpath[_POSIX_PATH_MAX];

  mutt_concat_path (fullpath, folder, path, sizeof (fullpath));

  if (stat (fullpath, &st) == 0)
    return (S_ISDIR (st.st_mode));
  else
    return 0;
}

static const char *folder_format_str (char *dest, size_t destlen, char op,
                                      const char *src, const char *fmt,
                                      const char *ifstring,
                                      const char *elsestring,
                                      unsigned long data, format_flag flags)
{
  char fn[SHORT_STRING], tmp[SHORT_STRING], permission[11];
  char date[16], *t_fmt;
  time_t tnow;
  FOLDER *folder = (FOLDER *) data;
  struct passwd *pw;
  struct group *gr;
  int optional = (flags & M_FORMAT_OPTIONAL);

  switch (op) {
  case 'C':
    snprintf (tmp, sizeof (tmp), "%%%sd", fmt);
    snprintf (dest, destlen, tmp, folder->num + 1);
    break;

  case 'd':
    if (folder->ff->st != NULL) {
      tnow = time (NULL);
      t_fmt =
        tnow - folder->ff->st->st_mtime <
        31536000 ? "%b %d %H:%M" : "%b %d  %Y";
      strftime (date, sizeof (date), t_fmt,
                localtime (&folder->ff->st->st_mtime));
      mutt_format_s (dest, destlen, fmt, date);
    }
    else
      mutt_format_s (dest, destlen, fmt, "");
    break;

  case 'f':
    {
      char *s;

#ifdef USE_IMAP
      if (folder->ff->imap)
        s = NONULL (folder->ff->desc);
      else
#endif
        s = NONULL (folder->ff->name);

      snprintf (fn, sizeof (fn), "%s%s", s,
                folder->ff->st ? (S_ISLNK (folder->ff->st->st_mode) ? "@" :
                                  (S_ISDIR (folder->ff->st->st_mode) ? "/" :
                                   ((folder->ff->st->st_mode & S_IXUSR) !=
                                    0 ? "*" : ""))) : "");

      mutt_format_s (dest, destlen, fmt, fn);
      break;
    }
  case 'F':
    if (folder->ff->st != NULL) {
      snprintf (permission, sizeof (permission), "%c%c%c%c%c%c%c%c%c%c",
                S_ISDIR (folder->ff->st->
                         st_mode) ? 'd' : (S_ISLNK (folder->ff->st->
                                                    st_mode) ? 'l' : '-'),
                (folder->ff->st->st_mode & S_IRUSR) != 0 ? 'r' : '-',
                (folder->ff->st->st_mode & S_IWUSR) != 0 ? 'w' : '-',
                (folder->ff->st->st_mode & S_ISUID) !=
                0 ? 's' : (folder->ff->st->st_mode & S_IXUSR) !=
                0 ? 'x' : '-',
                (folder->ff->st->st_mode & S_IRGRP) != 0 ? 'r' : '-',
                (folder->ff->st->st_mode & S_IWGRP) != 0 ? 'w' : '-',
                (folder->ff->st->st_mode & S_ISGID) !=
                0 ? 's' : (folder->ff->st->st_mode & S_IXGRP) !=
                0 ? 'x' : '-',
                (folder->ff->st->st_mode & S_IROTH) != 0 ? 'r' : '-',
                (folder->ff->st->st_mode & S_IWOTH) != 0 ? 'w' : '-',
                (folder->ff->st->st_mode & S_ISVTX) !=
                0 ? 't' : (folder->ff->st->st_mode & S_IXOTH) !=
                0 ? 'x' : '-');
      mutt_format_s (dest, destlen, fmt, permission);
    }
#ifdef USE_IMAP
    else if (folder->ff->imap) {
      /* mark folders with subfolders AND mail */
      snprintf (permission, sizeof (permission), "IMAP %c",
                (folder->ff->inferiors
                 && folder->ff->selectable) ? '+' : ' ');
      mutt_format_s (dest, destlen, fmt, permission);
    }
#endif
    else
      mutt_format_s (dest, destlen, fmt, "");
    break;

  case 'g':
    if (folder->ff->st != NULL) {
      if ((gr = getgrgid (folder->ff->st->st_gid)))
        mutt_format_s (dest, destlen, fmt, gr->gr_name);
      else {
        snprintf (tmp, sizeof (tmp), "%%%sld", fmt);
        snprintf (dest, destlen, tmp, folder->ff->st->st_gid);
      }
    }
    else
      mutt_format_s (dest, destlen, fmt, "");
    break;

  case 'l':
    if (folder->ff->st != NULL) {
      snprintf (tmp, sizeof (tmp), "%%%sd", fmt);
      snprintf (dest, destlen, tmp, folder->ff->st->st_nlink);
    }
    else
      mutt_format_s (dest, destlen, fmt, "");
    break;

  case 'N':
#ifdef USE_IMAP
    if (imap_is_magic (folder->ff->desc, NULL) == M_IMAP) {
      if (!optional) {
        snprintf (tmp, sizeof (tmp), "%%%sd", fmt);
        snprintf (dest, destlen, tmp, folder->ff->new);
      }
      else if (!folder->ff->new)
        optional = 0;
      break;
    }
#endif
    snprintf (tmp, sizeof (tmp), "%%%sc", fmt);
    snprintf (dest, destlen, tmp, folder->ff->new ? 'N' : ' ');
    break;

  case 's':
    if (folder->ff->st != NULL) {
      snprintf (tmp, sizeof (tmp), "%%%sld", fmt);
      snprintf (dest, destlen, tmp, (long) folder->ff->st->st_size);
    }
    else
      mutt_format_s (dest, destlen, fmt, "");
    break;

  case 't':
    snprintf (tmp, sizeof (tmp), "%%%sc", fmt);
    snprintf (dest, destlen, tmp, folder->ff->tagged ? '*' : ' ');
    break;

  case 'u':
    if (folder->ff->st != NULL) {
      if ((pw = getpwuid (folder->ff->st->st_uid)))
        mutt_format_s (dest, destlen, fmt, pw->pw_name);
      else {
        snprintf (tmp, sizeof (tmp), "%%%sld", fmt);
        snprintf (dest, destlen, tmp, folder->ff->st->st_uid);
      }
    }
    else
      mutt_format_s (dest, destlen, fmt, "");
    break;

  default:
    snprintf (tmp, sizeof (tmp), "%%%sc", fmt);
    snprintf (dest, destlen, tmp, op);
    break;
  }

  if (optional)
    mutt_FormatString (dest, destlen, ifstring, folder_format_str, data, 0);
  else if (flags & M_FORMAT_OPTIONAL)
    mutt_FormatString (dest, destlen, elsestring, folder_format_str, data, 0);

  return (src);
}

#ifdef USE_NNTP
static const char *newsgroup_format_str (char *dest, size_t destlen, char op,
                                         const char *src, const char *fmt,
                                         const char *ifstring,
                                         const char *elsestring,
                                         unsigned long data,
                                         format_flag flags)
{
  char fn[SHORT_STRING], tmp[SHORT_STRING];
  FOLDER *folder = (FOLDER *) data;

  switch (op) {
  case 'C':
    snprintf (tmp, sizeof (tmp), "%%%sd", fmt);
    snprintf (dest, destlen, tmp, folder->num + 1);
    break;

  case 'f':
    strncpy (fn, folder->ff->name, sizeof (fn) - 1);
    snprintf (tmp, sizeof (tmp), "%%%ss", fmt);
    snprintf (dest, destlen, tmp, fn);
    break;

  case 'N':
    snprintf (tmp, sizeof (tmp), "%%%sc", fmt);
    if (folder->ff->nd->subscribed)
      snprintf (dest, destlen, tmp, ' ');
    else
      snprintf (dest, destlen, tmp, folder->ff->new ? 'N' : 'u');
    break;

  case 'M':
    snprintf (tmp, sizeof (tmp), "%%%sc", fmt);
    if (folder->ff->nd->deleted)
      snprintf (dest, destlen, tmp, 'D');
    else
      snprintf (dest, destlen, tmp, folder->ff->nd->allowed ? ' ' : '-');
    break;

  case 's':
    if (flags & M_FORMAT_OPTIONAL) {
      if (folder->ff->nd->unread != 0)
        mutt_FormatString (dest, destlen, ifstring, newsgroup_format_str,
                           data, flags);
      else
        mutt_FormatString (dest, destlen, elsestring, newsgroup_format_str,
                           data, flags);
    }
    else if (Context && Context->data == folder->ff->nd) {
      snprintf (tmp, sizeof (tmp), "%%%sd", fmt);
      snprintf (dest, destlen, tmp, Context->unread);
    }
    else {
      snprintf (tmp, sizeof (tmp), "%%%sd", fmt);
      snprintf (dest, destlen, tmp, folder->ff->nd->unread);
    }
    break;

  case 'n':
    if (Context && Context->data == folder->ff->nd) {
      snprintf (tmp, sizeof (tmp), "%%%sd", fmt);
      snprintf (dest, destlen, tmp, Context->new);
    }
    else if (option (OPTMARKOLD) &&
             folder->ff->nd->lastCached >= folder->ff->nd->firstMessage &&
             folder->ff->nd->lastCached <= folder->ff->nd->lastMessage) {
      snprintf (tmp, sizeof (tmp), "%%%sd", fmt);
      snprintf (dest, destlen, tmp,
                folder->ff->nd->lastMessage - folder->ff->nd->lastCached);
    }
    else {
      snprintf (tmp, sizeof (tmp), "%%%sd", fmt);
      snprintf (dest, destlen, tmp, folder->ff->nd->unread);
    }
    break;

  case 'd':
    if (folder->ff->nd->desc != NULL) {
      snprintf (tmp, sizeof (tmp), "%%%ss", fmt);
      snprintf (dest, destlen, tmp, folder->ff->nd->desc);
    }
    else {
      snprintf (tmp, sizeof (tmp), "%%%ss", fmt);
      snprintf (dest, destlen, tmp, "");
    }
    break;
  }
  return (src);
}
#endif /* USE_NNTP */

static void add_folder (MUTTMENU * m, struct browser_state *state,
                        const char *name, const struct stat *s,
                        void *data, int new)
{
  if (state->entrylen == state->entrymax) {
    /* need to allocate more space */
    mem_realloc (&state->entry,
                  sizeof (struct folder_file) * (state->entrymax += 256));
    memset (&state->entry[state->entrylen], 0,
            sizeof (struct folder_file) * 256);
    if (m)
      m->data = state->entry;
  }

  if (s != NULL) {
    (state->entry)[state->entrylen].mode = s->st_mode;
    (state->entry)[state->entrylen].mtime = s->st_mtime;
    (state->entry)[state->entrylen].size = s->st_size;

    (state->entry)[state->entrylen].st = mem_malloc (sizeof (struct stat));
    memcpy ((state->entry)[state->entrylen].st, s, sizeof (struct stat));
  }

  (state->entry)[state->entrylen].new = new;
  (state->entry)[state->entrylen].name = str_dup (name);
  (state->entry)[state->entrylen].desc = str_dup (name);
#ifdef USE_IMAP
  (state->entry)[state->entrylen].imap = 0;
#endif
#ifdef USE_NNTP
  if (option (OPTNEWS))
    (state->entry)[state->entrylen].nd = (NNTP_DATA *) data;
#endif
  (state->entrylen)++;
}

static void init_state (struct browser_state *state, MUTTMENU * menu)
{
  state->entrylen = 0;
  state->entrymax = 256;
  state->entry =
    (struct folder_file *) mem_calloc (state->entrymax,
                                        sizeof (struct folder_file));
#ifdef USE_IMAP
  state->imap_browse = 0;
#endif
  if (menu)
    menu->data = state->entry;
}

/* get list of all files/newsgroups with mask */
static int examine_directory (MUTTMENU * menu, struct browser_state *state,
                              char *d, const char *prefix)
{
#ifdef USE_NNTP
  if (option (OPTNEWS)) {
    LIST *tmp;
    NNTP_DATA *data;
    NNTP_SERVER *news = CurrentNewsSrv;

/*  buffy_check (0); */
    init_state (state, menu);

    for (tmp = news->list; tmp; tmp = tmp->next) {
      if (!(data = (NNTP_DATA *) tmp->data))
        continue;
      nntp_sync_sidebar (data);
      if (prefix && *prefix && strncmp (prefix, data->group,
                                        str_len (prefix)) != 0)
        continue;
      if (!((regexec (Mask.rx, data->group, 0, NULL, 0) == 0) ^ Mask.not))
        continue;
      add_folder (menu, state, data->group, NULL, data, data->new);
    }
    sidebar_draw (CurrentMenu);
  }
  else
#endif /* USE_NNTP */
  {
    struct stat s;
    DIR *dp;
    struct dirent *de;
    char buffer[_POSIX_PATH_MAX + SHORT_STRING];
    int i = -1;

    while (stat (d, &s) == -1) {
      if (errno == ENOENT) {
        /* The last used directory is deleted, try to use the parent dir. */
        char *c = strrchr (d, '/');

        if (c && (c > d)) {
          *c = 0;
          continue;
        }
      }
      mutt_perror (d);
      return (-1);
    }

    if (!S_ISDIR (s.st_mode)) {
      mutt_error (_("%s is not a directory."), d);
      return (-1);
    }

    buffy_check (0);

    if ((dp = opendir (d)) == NULL) {
      mutt_perror (d);
      return (-1);
    }

    init_state (state, menu);

    while ((de = readdir (dp)) != NULL) {
      if (str_cmp (de->d_name, ".") == 0)
        continue;               /* we don't need . */

      if (prefix && *prefix
          && str_ncmp (prefix, de->d_name, str_len (prefix)) != 0)
        continue;
      if (!((regexec (Mask.rx, de->d_name, 0, NULL, 0) == 0) ^ Mask.not))
        continue;

      mutt_concat_path (buffer, d, de->d_name, sizeof (buffer));
      if (lstat (buffer, &s) == -1)
        continue;

      if ((!S_ISREG (s.st_mode)) && (!S_ISDIR (s.st_mode)) &&
          (!S_ISLNK (s.st_mode)))
        continue;

      i = buffy_lookup (buffer);
      add_folder (menu, state, de->d_name, &s, NULL, i >= 0 ? ((BUFFY*) Incoming->data[i])->new : 0);
    }
    closedir (dp);
  }
  sidebar_draw (CurrentMenu);
  browser_sort (state);
  return 0;
}

/* get list of mailboxes/subscribed newsgroups */
static int examine_mailboxes (MUTTMENU * menu, struct browser_state *state)
{
  struct stat s;
  char buffer[LONG_STRING];

#ifdef USE_NNTP
  if (option (OPTNEWS)) {
    LIST *tmp;
    NNTP_DATA *data;
    NNTP_SERVER *news = CurrentNewsSrv;

/*  buffy_check (0); */
    init_state (state, menu);

    for (tmp = news->list; tmp; tmp = tmp->next) {
      if ((data = (NNTP_DATA*) tmp->data) == NULL)
        continue;
      nntp_sync_sidebar (data);
      if ((data->new || (data->subscribed && 
                         (!option (OPTSHOWONLYUNREAD)|| data->unread))))
        add_folder (menu, state, data->group, NULL, data, data->new);
    }
    sidebar_draw (CurrentMenu);
  }
  else
#endif
  {
    int i = 0;
    BUFFY* tmp;

    if (!Incoming)
      return (-1);
    buffy_check (0);

    init_state (state, menu);

    for (i = 0; i < Incoming->length; i++) {
      tmp = (BUFFY*) Incoming->data[i];
      tmp->magic = mx_get_magic (tmp->path);
#ifdef USE_IMAP
      if (tmp->magic == M_IMAP) {
        add_folder (menu, state, tmp->path, NULL, NULL, tmp->new);
        continue;
      }
#endif
#ifdef USE_POP
      if (tmp->magic == M_POP) {
        add_folder (menu, state, tmp->path, NULL, NULL, tmp->new);
        continue;
      }
#endif
#ifdef USE_NNTP
      if (tmp->magic == M_NNTP) {
        add_folder (menu, state, tmp->path, NULL, NULL, tmp->new);
        continue;
      }
#endif
      if (lstat (tmp->path, &s) == -1)
        continue;

      if ((!S_ISREG (s.st_mode)) && (!S_ISDIR (s.st_mode)) &&
          (!S_ISLNK (s.st_mode)))
        continue;

      strfcpy (buffer, NONULL (tmp->path), sizeof (buffer));
      mutt_pretty_mailbox (buffer);

      add_folder (menu, state, buffer, &s, NULL, tmp->new);
    }
  }
  browser_sort (state);
  return 0;
}

static int select_file_search (MUTTMENU * menu, regex_t * re, int n)
{
#ifdef USE_NNTP
  if (option (OPTNEWS))
    return (regexec
            (re, ((struct folder_file *) menu->data)[n].desc, 0, NULL, 0));
#endif
  return (regexec
          (re, ((struct folder_file *) menu->data)[n].name, 0, NULL, 0));
}

static void folder_entry (char *s, size_t slen, MUTTMENU * menu, int num)
{
  FOLDER folder;

  folder.ff = &((struct folder_file *) menu->data)[num];
  folder.num = num;

#ifdef USE_NNTP
  if (option (OPTNEWS))
    mutt_FormatString (s, slen, NONULL (GroupFormat), newsgroup_format_str,
                       (unsigned long) &folder, M_FORMAT_ARROWCURSOR);
  else
#endif
    mutt_FormatString (s, slen, NONULL (FolderFormat), folder_format_str,
                       (unsigned long) &folder, M_FORMAT_ARROWCURSOR);
}

static void init_menu (struct browser_state *state, MUTTMENU * menu,
                       char *title, size_t titlelen, int buffy)
{
  char path[_POSIX_PATH_MAX];

  menu->max = state->entrylen;

  if (menu->current >= menu->max)
    menu->current = menu->max - 1;
  if (menu->current < 0)
    menu->current = 0;
  if (menu->top > menu->current)
    menu->top = 0;

  menu->tagged = 0;

#ifdef USE_NNTP
  if (option (OPTNEWS)) {
    if (buffy)
      snprintf (title, titlelen, "%s", _("Subscribed newsgroups"));
    else
      snprintf (title, titlelen, _("Newsgroups on server [%s]"),
                CurrentNewsSrv->conn->account.host);
  }
  else
#endif
  if (buffy)
    snprintf (title, titlelen, _("Mailboxes [%d]"), buffy_check (0));
  else {
    strfcpy (path, LastDir, sizeof (path));
    mutt_pretty_mailbox (path);
#ifdef USE_IMAP
    if (state->imap_browse && option (OPTIMAPLSUB))
      snprintf (title, titlelen, _("Subscribed [%s], File mask: %s"),
                path, NONULL (Mask.pattern));
    else
#endif
      snprintf (title, titlelen, _("Directory [%s], File mask: %s"),
                path, NONULL (Mask.pattern));
  }
  menu->redraw = REDRAW_FULL;
}

static int file_tag (MUTTMENU * menu, int n, int m)
{
  struct folder_file *ff = &(((struct folder_file *) menu->data)[n]);
  int ot;

  if (S_ISDIR (ff->mode)
      || (S_ISLNK (ff->mode) && link_is_dir (LastDir, ff->name))) {
    mutt_error _("Can't attach a directory!");

    return 0;
  }

  ot = ff->tagged;
  ff->tagged = (m >= 0 ? m : !ff->tagged);

  return ff->tagged - ot;
}

void _mutt_select_file (char *f, size_t flen, int flags, char ***files,
                        int *numfiles)
{
  char buf[_POSIX_PATH_MAX];
  char prefix[_POSIX_PATH_MAX] = "";
  char helpstr[SHORT_STRING];
  char title[STRING];
  struct browser_state state;
  MUTTMENU *menu;
  struct stat st;
  int i, killPrefix = 0;
  int multiple = (flags & M_SEL_MULTI) ? 1 : 0;
  int folder = (flags & M_SEL_FOLDER) ? 1 : 0;
  int buffy = (flags & M_SEL_BUFFY) ? 1 : 0;

  buffy = buffy && folder;

  memset (&state, 0, sizeof (struct browser_state));

  if (!folder)
    strfcpy (LastDirBackup, LastDir, sizeof (LastDirBackup));

#ifdef USE_NNTP
  if (option (OPTNEWS)) {
    if (*f)
      strfcpy (prefix, f, sizeof (prefix));
    else {
      LIST *list;

      /* default state for news reader mode is browse subscribed newsgroups */
      buffy = 0;
      for (list = CurrentNewsSrv->list; list; list = list->next) {
        NNTP_DATA *data = (NNTP_DATA *) list->data;

        if (data && data->subscribed) {
          buffy = 1;
          break;
        }
      }
    }
  }
  else
#endif
  if (*f) {
    mutt_expand_path (f, flen);
#ifdef USE_IMAP
    if (imap_is_magic (f, NULL) == M_IMAP) {
      init_state (&state, NULL);
      state.imap_browse = 1;
      if (!imap_browse (f, &state))
        strfcpy (LastDir, state.folder, sizeof (LastDir));
    }
    else {
#endif
      for (i = str_len (f) - 1; i > 0 && f[i] != '/'; i--);
      if (i > 0) {
        if (f[0] == '/') {
          if (i > sizeof (LastDir) - 1)
            i = sizeof (LastDir) - 1;
          strncpy (LastDir, f, i);
          LastDir[i] = 0;
        }
        else {
          getcwd (LastDir, sizeof (LastDir));
          str_cat (LastDir, sizeof (LastDir), "/");
          str_ncat (LastDir, sizeof (LastDir), f, i);
        }
      }
      else {
        if (f[0] == '/')
          strcpy (LastDir, "/");        /* __STRCPY_CHECKED__ */
        else
          getcwd (LastDir, sizeof (LastDir));
      }

      if (i <= 0 && f[0] != '/')
        strfcpy (prefix, f, sizeof (prefix));
      else
        strfcpy (prefix, f + i + 1, sizeof (prefix));
      killPrefix = 1;
#ifdef USE_IMAP
    }
#endif
  }
  else {
    if (!folder)
      getcwd (LastDir, sizeof (LastDir));
    else if (!LastDir[0])
      strfcpy (LastDir, NONULL (Maildir), sizeof (LastDir));

#ifdef USE_IMAP
    if (!buffy && imap_is_magic (LastDir, NULL) == M_IMAP) {
      init_state (&state, NULL);
      state.imap_browse = 1;
      imap_browse (LastDir, &state);
      browser_sort (&state);
    }
#endif
  }

  *f = 0;

  if (buffy) {
    if (examine_mailboxes (NULL, &state) == -1)
      goto bail;
  }
  else
#ifdef USE_IMAP
  if (!state.imap_browse)
#endif
    if (examine_directory (NULL, &state, LastDir, prefix) == -1)
      goto bail;

  menu = mutt_new_menu ();
  menu->menu = MENU_FOLDER;
  menu->make_entry = folder_entry;
  menu->search = select_file_search;
  menu->title = title;
  menu->data = state.entry;
  if (multiple)
    menu->tag = file_tag;

  menu->help = mutt_compile_help (helpstr, sizeof (helpstr), MENU_FOLDER,
#ifdef USE_NNTP
                                  (option (OPTNEWS)) ? FolderNewsHelp :
#endif
                                  FolderHelp);

  init_menu (&state, menu, title, sizeof (title), buffy);

  FOREVER {
    switch (i = mutt_menuLoop (menu)) {
    case OP_GENERIC_SELECT_ENTRY:

      if (!state.entrylen) {
        mutt_error _("No files match the file mask");

        break;
      }

      if (S_ISDIR (state.entry[menu->current].mode) ||
          (S_ISLNK (state.entry[menu->current].mode) &&
           link_is_dir (LastDir, state.entry[menu->current].name))
#ifdef USE_IMAP
          || state.entry[menu->current].inferiors
#endif
        ) {
        /* make sure this isn't a MH or maildir mailbox */
        if (buffy) {
          strfcpy (buf, state.entry[menu->current].name, sizeof (buf));
          mutt_expand_path (buf, sizeof (buf));
        }
#ifdef USE_IMAP
        else if (state.imap_browse) {
          strfcpy (buf, state.entry[menu->current].name, sizeof (buf));
        }
#endif
        else
          mutt_concat_path (buf, LastDir, state.entry[menu->current].name,
                            sizeof (buf));

        if ((mx_get_magic (buf) <= 0)
#ifdef USE_IMAP
            || state.entry[menu->current].inferiors
#endif
          ) {
          char OldLastDir[_POSIX_PATH_MAX];

          /* save the old directory */
          strfcpy (OldLastDir, LastDir, sizeof (OldLastDir));

          if (str_cmp (state.entry[menu->current].name, "..") == 0) {
            if (str_cmp ("..", LastDir + str_len (LastDir) - 2) == 0)
              strcat (LastDir, "/..");  /* __STRCAT_CHECKED__ */
            else {
              char *p = strrchr (LastDir + 1, '/');

              if (p)
                *p = 0;
              else {
                if (LastDir[0] == '/')
                  LastDir[1] = 0;
                else
                  strcat (LastDir, "/..");      /* __STRCAT_CHECKED__ */
              }
            }
          }
          else if (buffy) {
            strfcpy (LastDir, state.entry[menu->current].name,
                     sizeof (LastDir));
            mutt_expand_path (LastDir, sizeof (LastDir));
          }
#ifdef USE_IMAP
          else if (state.imap_browse) {
            int n;
            ciss_url_t url;

            strfcpy (LastDir, state.entry[menu->current].name,
                     sizeof (LastDir));
            /* tack on delimiter here */
            n = str_len (LastDir) + 1;

            /* special case "" needs no delimiter */
            url_parse_ciss (&url, state.entry[menu->current].name);
            if (url.path &&
                (state.entry[menu->current].delim != '\0') &&
                (n < sizeof (LastDir))) {
              LastDir[n] = '\0';
              LastDir[n - 1] = state.entry[menu->current].delim;
            }
          }
#endif
          else {
            char tmp[_POSIX_PATH_MAX];

            mutt_concat_path (tmp, LastDir, state.entry[menu->current].name,
                              sizeof (tmp));
            strfcpy (LastDir, tmp, sizeof (LastDir));
          }

          destroy_state (&state);
          if (killPrefix) {
            prefix[0] = 0;
            killPrefix = 0;
          }
          buffy = 0;
#ifdef USE_IMAP
          if (state.imap_browse) {
            init_state (&state, NULL);
            state.imap_browse = 1;
            imap_browse (LastDir, &state);
            browser_sort (&state);
            menu->data = state.entry;
          }
          else
#endif
          if (examine_directory (menu, &state, LastDir, prefix) == -1) {
            /* try to restore the old values */
            strfcpy (LastDir, OldLastDir, sizeof (LastDir));
            if (examine_directory (menu, &state, LastDir, prefix) == -1) {
              strfcpy (LastDir, NONULL (Homedir), sizeof (LastDir));
              goto bail;
            }
          }
          menu->current = 0;
          menu->top = 0;
          init_menu (&state, menu, title, sizeof (title), buffy);
          break;
        }
      }

#ifdef USE_NNTP
      if (buffy || option (OPTNEWS))    /* news have not path */
#else
      if (buffy)
#endif
      {
        strfcpy (f, state.entry[menu->current].name, flen);
        mutt_expand_path (f, flen);
      }
#ifdef USE_IMAP
      else if (state.imap_browse)
        strfcpy (f, state.entry[menu->current].name, flen);
#endif
      else
        mutt_concat_path (f, LastDir, state.entry[menu->current].name, flen);

      /* Fall through to OP_EXIT */

    case OP_EXIT:

      if (multiple) {
        char **tfiles;
        int i, j;

        if (menu->tagged) {
          *numfiles = menu->tagged;
          tfiles = mem_calloc (*numfiles, sizeof (char *));
          for (i = 0, j = 0; i < state.entrylen; i++) {
            struct folder_file ff = state.entry[i];
            char full[_POSIX_PATH_MAX];

            if (ff.tagged) {
              mutt_concat_path (full, LastDir, ff.name, sizeof (full));
              mutt_expand_path (full, sizeof (full));
              tfiles[j++] = str_dup (full);
            }
          }
          *files = tfiles;
        }
        else if (f[0]) {        /* no tagged entries. return selected entry */
          *numfiles = 1;
          tfiles = mem_calloc (*numfiles, sizeof (char *));
          mutt_expand_path (f, flen);
          tfiles[0] = str_dup (f);
          *files = tfiles;
        }
      }

      destroy_state (&state);
      mutt_menuDestroy (&menu);
      goto bail;

    case OP_BROWSER_TELL:
      if (state.entrylen)
        mutt_message ("%s", state.entry[menu->current].name);
      break;

#ifdef USE_IMAP
    case OP_BROWSER_TOGGLE_LSUB:
      if (option (OPTIMAPLSUB)) {
        unset_option (OPTIMAPLSUB);
      }
      else {
        set_option (OPTIMAPLSUB);
      }
      mutt_ungetch (0, OP_CHECK_NEW);
      break;

    case OP_CREATE_MAILBOX:
      if (!state.imap_browse)
        mutt_error (_("Create is only supported for IMAP mailboxes"));
      else {
        imap_mailbox_create (LastDir);
        /* TODO: find a way to detect if the new folder would appear in
         *   this window, and insert it without starting over. */
        destroy_state (&state);
        init_state (&state, NULL);
        state.imap_browse = 1;
        imap_browse (LastDir, &state);
        browser_sort (&state);
        menu->data = state.entry;
        menu->current = 0;
        menu->top = 0;
        init_menu (&state, menu, title, sizeof (title), buffy);
        MAYBE_REDRAW (menu->redraw);
      }
      break;

    case OP_RENAME_MAILBOX:
      if (!state.entry[menu->current].imap)
        mutt_error (_("Rename is only supported for IMAP mailboxes"));
      else {
        int nentry = menu->current;

        if (imap_mailbox_rename (state.entry[nentry].name) >= 0) {
          destroy_state (&state);
          init_state (&state, NULL);
          state.imap_browse = 1;
          imap_browse (LastDir, &state);
          browser_sort (&state);
          menu->data = state.entry;
          menu->current = 0;
          menu->top = 0;
          init_menu (&state, menu, title, sizeof (title), buffy);
          MAYBE_REDRAW (menu->redraw);
        }
      }
      break;

    case OP_DELETE_MAILBOX:
      if (!state.entry[menu->current].imap)
        mutt_error (_("Delete is only supported for IMAP mailboxes"));
      else {
        char msg[SHORT_STRING];
        IMAP_MBOX mx;
        int nentry = menu->current;

        imap_parse_path (state.entry[nentry].name, &mx);
        snprintf (msg, sizeof (msg), _("Really delete mailbox \"%s\"?"),
                  mx.mbox);
        if (mutt_yesorno (msg, M_NO) == M_YES) {
          if (!imap_delete_mailbox (Context, mx)) {
            /* free the mailbox from the browser */
            mem_free (&((state.entry)[nentry].name));
            mem_free (&((state.entry)[nentry].desc));
            /* and move all other entries up */
            if (nentry + 1 < state.entrylen)
              memmove (state.entry + nentry, state.entry + nentry + 1,
                       sizeof (struct folder_file) * (state.entrylen -
                                                      (nentry + 1)));
            state.entrylen--;
            mutt_message _("Mailbox deleted.");

            init_menu (&state, menu, title, sizeof (title), buffy);
            MAYBE_REDRAW (menu->redraw);
          }
        }
        else
          mutt_message _("Mailbox not deleted.");
        mem_free (&mx.mbox);
      }
      break;
#endif

    case OP_CHANGE_DIRECTORY:

#ifdef USE_NNTP
      if (option (OPTNEWS))
        break;
#endif

      strfcpy (buf, LastDir, sizeof (buf));
#ifdef USE_IMAP
      if (!state.imap_browse)
#endif
      {
        /* add '/' at the end of the directory name if not already there */
        int len = str_len (LastDir);

        if (len && LastDir[len - 1] != '/' && sizeof (buf) > len)
          buf[len] = '/';
      }

      if (mutt_get_field (_("Chdir to: "), buf, sizeof (buf), M_FILE) == 0 &&
          buf[0]) {
        buffy = 0;
        mutt_expand_path (buf, sizeof (buf));
#ifdef USE_IMAP
        if (imap_is_magic (buf, NULL) == M_IMAP) {
          strfcpy (LastDir, buf, sizeof (LastDir));
          destroy_state (&state);
          init_state (&state, NULL);
          state.imap_browse = 1;
          imap_browse (LastDir, &state);
          browser_sort (&state);
          menu->data = state.entry;
          menu->current = 0;
          menu->top = 0;
          init_menu (&state, menu, title, sizeof (title), buffy);
        }
        else
#endif
        if (stat (buf, &st) == 0) {
          if (S_ISDIR (st.st_mode)) {
            destroy_state (&state);
            if (examine_directory (menu, &state, buf, prefix) == 0)
              strfcpy (LastDir, buf, sizeof (LastDir));
            else {
              mutt_error _("Error scanning directory.");

              if (examine_directory (menu, &state, LastDir, prefix) == -1) {
                mutt_menuDestroy (&menu);
                goto bail;
              }
            }
            menu->current = 0;
            menu->top = 0;
            init_menu (&state, menu, title, sizeof (title), buffy);
          }
          else
            mutt_error (_("%s is not a directory."), buf);
        }
        else
          mutt_perror (buf);
      }
      MAYBE_REDRAW (menu->redraw);
      break;

    case OP_ENTER_MASK:

      strfcpy (buf, NONULL (Mask.pattern), sizeof (buf));
      if (mutt_get_field (_("File Mask: "), buf, sizeof (buf), 0) == 0) {
        regex_t *rx = (regex_t *) mem_malloc (sizeof (regex_t));
        char *s = buf;
        int not = 0, err;

        buffy = 0;
        /* assume that the user wants to see everything */
        if (!buf[0])
          strfcpy (buf, ".", sizeof (buf));
        SKIPWS (s);
        if (*s == '!') {
          s++;
          SKIPWS (s);
          not = 1;
        }

        if ((err = REGCOMP (rx, s, REG_NOSUB)) != 0) {
          regerror (err, rx, buf, sizeof (buf));
          regfree (rx);
          mem_free (&rx);
          mutt_error ("%s", buf);
        }
        else {
          str_replace (&Mask.pattern, buf);
          regfree (Mask.rx);
          mem_free (&Mask.rx);
          Mask.rx = rx;
          Mask.not = not;

          destroy_state (&state);
#ifdef USE_IMAP
          if (state.imap_browse) {
            init_state (&state, NULL);
            state.imap_browse = 1;
            imap_browse (LastDir, &state);
            browser_sort (&state);
            menu->data = state.entry;
            init_menu (&state, menu, title, sizeof (title), buffy);
          }
          else
#endif
          if (examine_directory (menu, &state, LastDir, NULL) == 0)
            init_menu (&state, menu, title, sizeof (title), buffy);
          else {
            mutt_error _("Error scanning directory.");

            mutt_menuDestroy (&menu);
            goto bail;
          }
          killPrefix = 0;
          if (!state.entrylen) {
            mutt_error _("No files match the file mask");

            break;
          }
        }
      }
      MAYBE_REDRAW (menu->redraw);
      break;

    case OP_SORT:
    case OP_SORT_REVERSE:

      {
        int resort = 1;
        int reverse = (i == OP_SORT_REVERSE);

        switch (mutt_multi_choice ((reverse) ?
                                   _
                                   ("Reverse sort by (d)ate, (a)lpha, si(z)e or do(n)'t sort? ")
                                   :
                                   _
                                   ("Sort by (d)ate, (a)lpha, si(z)e or do(n)'t sort? "),
                                   _("dazn"))) {
        case -1:               /* abort */
          resort = 0;
          break;

        case 1:                /* (d)ate */
          BrowserSort = SORT_DATE;
          break;

        case 2:                /* (a)lpha */
          BrowserSort = SORT_SUBJECT;
          break;

        case 3:                /* si(z)e */
          BrowserSort = SORT_SIZE;
          break;

        case 4:                /* do(n)'t sort */
          BrowserSort = SORT_ORDER;
          resort = 0;
          break;
        }
        if (resort) {
          BrowserSort |= reverse ? SORT_REVERSE : 0;
          browser_sort (&state);
          menu->redraw = REDRAW_FULL;
        }
        break;
      }

    case OP_TOGGLE_MAILBOXES:
      buffy = 1 - buffy;

    case OP_CHECK_NEW:
      destroy_state (&state);
      prefix[0] = 0;
      killPrefix = 0;

      if (buffy) {
        if (examine_mailboxes (menu, &state) == -1)
          goto bail;
      }
#ifdef USE_IMAP
      else if (imap_is_magic (LastDir, NULL) == M_IMAP) {
        init_state (&state, NULL);
        state.imap_browse = 1;
        imap_browse (LastDir, &state);
        browser_sort (&state);
        menu->data = state.entry;
      }
#endif
      else if (examine_directory (menu, &state, LastDir, prefix) == -1)
        goto bail;
      init_menu (&state, menu, title, sizeof (title), buffy);
      break;

    case OP_BUFFY_LIST:
      if (option (OPTFORCEBUFFYCHECK))
        buffy_check (1);
      buffy_list ();
      break;

    case OP_BROWSER_NEW_FILE:

      snprintf (buf, sizeof (buf), "%s/", LastDir);
      if (mutt_get_field (_("New file name: "), buf, sizeof (buf), M_FILE) ==
          0) {
        strfcpy (f, buf, flen);
        destroy_state (&state);
        mutt_menuDestroy (&menu);
        goto bail;
      }
      MAYBE_REDRAW (menu->redraw);
      break;

    case OP_BROWSER_VIEW_FILE:
      if (!state.entrylen) {
        mutt_error _("No files match the file mask");

        break;
      }

#ifdef USE_IMAP
      if (state.entry[menu->current].selectable) {
        strfcpy (f, state.entry[menu->current].name, flen);
        destroy_state (&state);
        mutt_menuDestroy (&menu);
        goto bail;
      }
      else
#endif
      if (S_ISDIR (state.entry[menu->current].mode) ||
            (S_ISLNK (state.entry[menu->current].mode) &&
               link_is_dir (LastDir, state.entry[menu->current].name))) {
        mutt_error _("Can't view a directory");

        break;
      }
      else {
        BODY *b;
        char buf[_POSIX_PATH_MAX];

        mutt_concat_path (buf, LastDir, state.entry[menu->current].name,
                          sizeof (buf));
        b = mutt_make_file_attach (buf);
        if (b != NULL) {
          mutt_view_attachment (NULL, b, M_REGULAR, NULL, NULL, 0);
          mutt_free_body (&b);
          menu->redraw = REDRAW_FULL;
        }
        else
          mutt_error _("Error trying to view file");
      }
      break;

#ifdef USE_NNTP
    case OP_CATCHUP:
    case OP_UNCATCHUP:
      if (option (OPTNEWS)) {
        struct folder_file *f = &state.entry[menu->current];
        NNTP_DATA *nd;

        if (i == OP_CATCHUP)
          nd = mutt_newsgroup_catchup (CurrentNewsSrv, f->name);
        else
          nd = mutt_newsgroup_uncatchup (CurrentNewsSrv, f->name);

        if (nd) {
/*	    FOLDER folder;
	    struct folder_file ff;
	    char buffer[_POSIX_PATH_MAX + SHORT_STRING];

	    folder.ff = &ff;
	    folder.ff->name = f->name;
	    folder.ff->st = NULL;
	    folder.ff->is_new = nd->new;
	    folder.ff->nd = nd;
	    mem_free (&f->desc);
	    mutt_FormatString (buffer, sizeof (buffer), NONULL(GroupFormat),
		  newsgroup_format_str, (unsigned long) &folder,
		  M_FORMAT_ARROWCURSOR);
	    f->desc = str_dup (buffer); */
          if (menu->current + 1 < menu->max)
            menu->current++;
          menu->redraw = REDRAW_MOTION_RESYNCH;
        }
      }
      break;

    case OP_LOAD_ACTIVE:
      if (!option (OPTNEWS))
        break;

      {
        LIST *tmp;
        NNTP_DATA *data;

        for (tmp = CurrentNewsSrv->list; tmp; tmp = tmp->next) {
          if ((data = (NNTP_DATA *) tmp->data))
            data->deleted = 1;
        }
      }
      nntp_get_active (CurrentNewsSrv);

      destroy_state (&state);
      if (buffy)
        examine_mailboxes (menu, &state);
      else
        examine_directory (menu, &state, NULL, NULL);
      init_menu (&state, menu, title, sizeof (title), buffy);
      break;
#endif /* USE_NNTP */

#if defined USE_IMAP || defined USE_NNTP
    case OP_BROWSER_SUBSCRIBE:
    case OP_BROWSER_UNSUBSCRIBE:
#endif
#ifdef USE_NNTP
    case OP_SUBSCRIBE_PATTERN:
    case OP_UNSUBSCRIBE_PATTERN:
      if (option (OPTNEWS)) {
        regex_t *rx = (regex_t *) mem_malloc (sizeof (regex_t));
        char *s = buf;
        int j = menu->current;
        NNTP_DATA *nd;
        NNTP_SERVER *news = CurrentNewsSrv;

        if (i == OP_SUBSCRIBE_PATTERN || i == OP_UNSUBSCRIBE_PATTERN) {
          char tmp[STRING];
          int err;

          buf[0] = 0;
          if (i == OP_SUBSCRIBE_PATTERN)
            snprintf (tmp, sizeof (tmp), _("Subscribe pattern: "));
          else
            snprintf (tmp, sizeof (tmp), _("Unsubscribe pattern: "));
          if (mutt_get_field (tmp, buf, sizeof (buf), 0) != 0 || !buf[0]) {
            mem_free (&rx);
            break;
          }

          if ((err = REGCOMP (rx, s, REG_NOSUB)) != 0) {
            regerror (err, rx, buf, sizeof (buf));
            regfree (rx);
            mem_free (&rx);
            mutt_error ("%s", buf);
            break;
          }
          menu->redraw = REDRAW_FULL;
          j = 0;
        }
        else if (!state.entrylen) {
          mutt_error _("No newsgroups match the mask");

          break;
        }

        for (; j < state.entrylen; j++) {
          struct folder_file *f = &state.entry[j];

          if (i == OP_BROWSER_SUBSCRIBE || i == OP_BROWSER_UNSUBSCRIBE ||
              regexec (rx, f->name, 0, NULL, 0) == 0) {
            if (i == OP_BROWSER_SUBSCRIBE || i == OP_SUBSCRIBE_PATTERN)
              nd = mutt_newsgroup_subscribe (news, f->name);
            else
              nd = mutt_newsgroup_unsubscribe (news, f->name);
/*	      if (nd)
	      {
		FOLDER folder;
		char buffer[_POSIX_PATH_MAX + SHORT_STRING];

		folder.name = f->name;
		folder.f = NULL;
		folder.new = nd->new;
		folder.nd = nd;
		mem_free (&f->desc);
		mutt_FormatString (buffer, sizeof (buffer), NONULL(GroupFormat),
			newsgroup_format_str, (unsigned long) &folder,
			M_FORMAT_ARROWCURSOR);
		f->desc = str_dup (buffer);
	      } */
          }
          if (i == OP_BROWSER_SUBSCRIBE || i == OP_BROWSER_UNSUBSCRIBE) {
            if (menu->current + 1 < menu->max)
              menu->current++;
            menu->redraw = REDRAW_MOTION_RESYNCH;
            break;
          }
        }
        if (i == OP_SUBSCRIBE_PATTERN) {
          LIST *grouplist = NULL;

          if (news)
            grouplist = news->list;
          for (; grouplist; grouplist = grouplist->next) {
            nd = (NNTP_DATA *) grouplist->data;
            if (nd && nd->group && !nd->subscribed) {
              if (regexec (rx, nd->group, 0, NULL, 0) == 0) {
                mutt_newsgroup_subscribe (news, nd->group);
                add_folder (menu, &state, nd->group, NULL, nd, nd->new);
              }
            }
          }
          init_menu (&state, menu, title, sizeof (title), buffy);
        }
        mutt_newsrc_update (news);
        nntp_clear_cacheindex (news);
        if (i != OP_BROWSER_SUBSCRIBE && i != OP_BROWSER_UNSUBSCRIBE)
          regfree (rx);
        mem_free (&rx);
      }
#ifdef USE_IMAP
      else
#endif /* USE_IMAP && USE_NNTP */
#endif /* USE_NNTP */
#ifdef USE_IMAP
      {
        if (i == OP_BROWSER_SUBSCRIBE)
          imap_subscribe (state.entry[menu->current].name, 1);
        else
          imap_subscribe (state.entry[menu->current].name, 0);
      }
#endif /* USE_IMAP */
    }
  }

bail:

  if (!folder)
    strfcpy (LastDir, LastDirBackup, sizeof (LastDir));

}
