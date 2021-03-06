/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-9 Brandon Long <blong@fiction.net>
 * Copyright (C) 1999-2002 Brendan Cully <brendan@kublai.com>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

/* Mutt browser support routines */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <ctype.h>

#include "lib/mem.h"
#include "lib/str.h"
#include "lib/intl.h"
#include "lib/debug.h"

#include "mutt.h"
#include "ascii.h"
#include "enter.h"
#include "imap_private.h"

/* -- forward declarations -- */
static int browse_add_list_result (IMAP_DATA * idata, const char *cmd,
                                   struct browser_state *state,
                                   short isparent);
static void imap_add_folder (char delim, char *folder, int noselect,
                             int noinferiors, struct browser_state *state,
                             short isparent);
static int compare_names (struct folder_file *a, struct folder_file *b);
static int browse_get_namespace (IMAP_DATA * idata, char *nsbuf, int nsblen,
                                 IMAP_NAMESPACE_INFO * nsi, int nsilen,
                                 int *nns);
static int browse_verify_namespace (IMAP_DATA * idata,
                                    IMAP_NAMESPACE_INFO * nsi, int nns);

/* imap_browse: IMAP hook into the folder browser, fills out browser_state,
 *   given a current folder to browse */
int imap_browse (char *path, struct browser_state *state)
{
  IMAP_DATA *idata;
  char buf[LONG_STRING];
  char buf2[LONG_STRING];
  char nsbuf[LONG_STRING];
  char mbox[LONG_STRING];
  char list_cmd[5];
  IMAP_NAMESPACE_INFO nsi[16];
  int home_namespace = 0;
  int n;
  int i;
  int nsup;
  char ctmp;
  int nns = 0;
  char *cur_folder;
  short showparents = 0;
  int noselect;
  int noinferiors;
  int save_lsub;
  IMAP_MBOX mx;

  if (imap_parse_path (path, &mx)) {
    mutt_error (_("%s is an invalid IMAP path"), path);
    return -1;
  }

  save_lsub = option (OPTIMAPCHECKSUBSCRIBED);
  unset_option (OPTIMAPCHECKSUBSCRIBED);
  strfcpy (list_cmd, option (OPTIMAPLSUB) ? "LSUB" : "LIST",
           sizeof (list_cmd));

  if (!(idata = imap_conn_find (&(mx.account), 0)))
    goto fail;

  if (!mx.mbox) {
    home_namespace = 1;
    mbox[0] = '\0';             /* Do not replace "" with "INBOX" here */
    mx.mbox = str_dup (ImapHomeNamespace);
    if (mutt_bit_isset (idata->capabilities, NAMESPACE)) {
      mutt_message _("Getting namespaces...");

      if (browse_get_namespace (idata, nsbuf, sizeof (nsbuf),
                                nsi, sizeof (nsi), &nns) != 0)
        goto fail;
      if (browse_verify_namespace (idata, nsi, nns) != 0)
        goto fail;
    }
  }

  mutt_message _("Getting folder list...");

  /* skip check for parents when at the root */
  if (mx.mbox && mx.mbox[0] != '\0') {
    imap_fix_path (idata, mx.mbox, mbox, sizeof (mbox));
    imap_munge_mbox_name (buf, sizeof (buf), mbox);
    imap_unquote_string (buf);  /* As kludgy as it gets */
    mbox[sizeof (mbox) - 1] = '\0';
    strncpy (mbox, buf, sizeof (mbox) - 1);
    n = str_len (mbox);

    debug_print (3, ("mbox: %s\n", mbox));

    /* if our target exists and has inferiors, enter it if we
     * aren't already going to */
    if (mbox[n - 1] != idata->delim) {
      snprintf (buf, sizeof (buf), "%s \"\" \"%s\"", list_cmd, mbox);
      imap_cmd_start (idata, buf);
      do {
        if (imap_parse_list_response (idata, &cur_folder, &noselect,
                                      &noinferiors, &idata->delim) != 0)
          goto fail;

        if (cur_folder) {
          imap_unmunge_mbox_name (cur_folder);

          if (!noinferiors && cur_folder[0] &&
              (n = str_len (mbox)) < LONG_STRING - 1) {
            mbox[n++] = idata->delim;
            mbox[n] = '\0';
          }
        }
      }
      while (ascii_strncmp (idata->cmd.buf, idata->cmd.seq, SEQLEN));
    }

    /* if we're descending a folder, mark it as current in browser_state */
    if (mbox[n - 1] == idata->delim) {
      /* don't show parents in the home namespace */
      if (!home_namespace)
        showparents = 1;
      imap_qualify_path (buf, sizeof (buf), &mx, mbox);
      state->folder = str_dup (buf);
      n--;
    }

    /* Find superiors to list
     * Note: UW-IMAP servers return folder + delimiter when asked to list
     *  folder + delimiter. Cyrus servers don't. So we ask for folder,
     *  and tack on delimiter ourselves.
     * Further note: UW-IMAP servers return nothing when asked for 
     *  NAMESPACES without delimiters at the end. Argh! */
    for (n--; n >= 0 && mbox[n] != idata->delim; n--);
    if (n > 0) {                /* "aaaa/bbbb/" -> "aaaa" */
      /* forget the check, it is too delicate (see above). Have we ever
       * had the parent not exist? */
      ctmp = mbox[n];
      mbox[n] = '\0';

      if (showparents) {
        debug_print (3, ("adding parent %s\n", mbox));
        imap_add_folder (idata->delim, mbox, 1, 0, state, 1);
      }

      /* if our target isn't a folder, we are in our superior */
      if (!state->folder) {
        /* store folder with delimiter */
        mbox[n++] = ctmp;
        ctmp = mbox[n];
        mbox[n] = '\0';
        imap_qualify_path (buf, sizeof (buf), &mx, mbox);
        state->folder = str_dup (buf);
      }
      mbox[n] = ctmp;
    }
    /* "/bbbb/" -> add  "/", "aaaa/" -> add "" */
    else {
      char relpath[2];

      /* folder may be "/" */
      snprintf (relpath, sizeof (relpath), "%c", n < 0 ? '\0' : idata->delim);
      if (showparents)
        imap_add_folder (idata->delim, relpath, 1, 0, state, 1);
      if (!state->folder) {
        imap_qualify_path (buf, sizeof (buf), &mx, relpath);
        state->folder = str_dup (buf);
      }
    }
  }

  /* no namespace, no folder: set folder to host only */
  if (!state->folder) {
    imap_qualify_path (buf, sizeof (buf), &mx, NULL);
    state->folder = str_dup (buf);
  }

  if (home_namespace && mbox[0] != '\0') {
    /* Listing the home namespace, so INBOX should be included. Home 
     * namespace is not "", so we have to list it explicitly. We ask the 
     * server to see if it has descendants. */
    debug_print (3, ("adding INBOX\n"));
    if (browse_add_list_result (idata, "LIST \"\" \"INBOX\"", state, 0))
      goto fail;
  }

  nsup = state->entrylen;

  debug_print (3, ("Quoting mailbox scan: %s:\n", mbox));
  snprintf (buf, sizeof (buf), "%s%%", mbox);
  imap_quote_string (buf2, sizeof (buf2), buf);
  debug_print (3, ("%s\n", buf2));
  snprintf (buf, sizeof (buf), "%s \"\" %s", list_cmd, buf2);
  if (browse_add_list_result (idata, buf, state, 0))
    goto fail;

  if (!state->entrylen) {
    mutt_error _("No such folder");

    goto fail;
  }

  mutt_clear_error ();

  qsort (&(state->entry[nsup]), state->entrylen - nsup,
         sizeof (state->entry[0]),
         (int (*)(const void *, const void *)) compare_names);
  if (home_namespace) {         /* List additional namespaces */
    for (i = 0; i < nns; i++)
      if (nsi[i].listable && !nsi[i].home_namespace) {
        imap_add_folder (nsi[i].delim, nsi[i].prefix, nsi[i].noselect,
                         nsi[i].noinferiors, state, 0);
        debug_print (3, ("adding namespace: %s\n", nsi[i].prefix));
      }
  }

  if (save_lsub)
    set_option (OPTIMAPCHECKSUBSCRIBED);

  mem_free (&mx.mbox);
  return 0;

fail:
  if (save_lsub)
    set_option (OPTIMAPCHECKSUBSCRIBED);
  mem_free (&mx.mbox);
  return -1;
}

/* imap_mailbox_create: Prompt for a new mailbox name, and try to create it */
int imap_mailbox_create (const char *folder)
{
  IMAP_DATA *idata;
  IMAP_MBOX mx;
  char buf[LONG_STRING];
  short n;

  if (imap_parse_path (folder, &mx) < 0) {
    debug_print (1, ("Bad starting path %s\n", folder));
    return -1;
  }

  if (!(idata = imap_conn_find (&mx.account, M_IMAP_CONN_NONEW))) {
    debug_print (1, ("Couldn't find open connection to %s\n", mx.account.host));
    goto fail;
  }

  strfcpy (buf, NONULL (mx.mbox), sizeof (buf));

  /* append a delimiter if necessary */
  n = str_len (buf);
  if (n && (n < sizeof (buf) - 1) && (buf[n - 1] != idata->delim)) {
    buf[n++] = idata->delim;
    buf[n] = '\0';
  }

  if (mutt_get_field (_("Create mailbox: "), buf, sizeof (buf), M_FILE) < 0)
    goto fail;

  if (!str_len (buf)) {
    mutt_error (_("Mailbox must have a name."));
    mutt_sleep (1);
    goto fail;
  }

  if (imap_create_mailbox (idata, buf) < 0)
    goto fail;

  mutt_message _("Mailbox created.");

  mutt_sleep (0);

  mem_free (&mx.mbox);
  return 0;

fail:
  mem_free (&mx.mbox);
  return -1;
}

int imap_mailbox_rename (const char *mailbox)
{
  IMAP_DATA *idata;
  IMAP_MBOX mx;
  char buf[LONG_STRING];
  char newname[SHORT_STRING];

  if (imap_parse_path (mailbox, &mx) < 0) {
    debug_print (1, ("Bad source mailbox %s\n", mailbox));
    return -1;
  }

  if (!(idata = imap_conn_find (&mx.account, M_IMAP_CONN_NONEW))) {
    debug_print (1, ("Couldn't find open connection to %s\n", mx.account.host));
    goto fail;
  }

  snprintf (buf, sizeof (buf), _("Rename mailbox %s to: "), mx.mbox);

  if (mutt_get_field (buf, newname, sizeof (newname), M_FILE) < 0)
    goto fail;

  if (!str_len (newname)) {
    mutt_error (_("Mailbox must have a name."));
    mutt_sleep (1);
    goto fail;
  }

  if (imap_rename_mailbox (idata, &mx, newname) < 0) {
    mutt_error (_("Rename failed: %s"), imap_get_qualifier (idata->cmd.buf));
    mutt_sleep (1);
    goto fail;
  }

  mutt_message (_("Mailbox renamed."));
  mutt_sleep (0);

  mem_free (&mx.mbox);
  return 0;

fail:
  mem_free (&mx.mbox);
  return -1;
}

static int browse_add_list_result (IMAP_DATA * idata, const char *cmd,
                                   struct browser_state *state,
                                   short isparent)
{
  char *name;
  int noselect;
  int noinferiors;
  IMAP_MBOX mx;

  if (imap_parse_path (state->folder, &mx)) {
    debug_print (2, ("current folder %s makes no sense\n", state->folder));
    return -1;
  }

  imap_cmd_start (idata, cmd);

  do {
    if (imap_parse_list_response (idata, &name, &noselect, &noinferiors,
                                  &idata->delim) != 0) {
      mem_free (&mx.mbox);
      return -1;
    }

    if (name) {
      /* Let a parent folder never be selectable for navigation */
      if (isparent)
        noselect = 1;
      /* prune current folder from output */
      if (isparent || str_ncmp (name, mx.mbox, str_len (name)))
        imap_add_folder (idata->delim, name, noselect, noinferiors, state,
                         isparent);
    }
  }
  while ((ascii_strncmp (idata->cmd.buf, idata->cmd.seq, SEQLEN) != 0));

  mem_free (&mx.mbox);
  return 0;
}

/* imap_add_folder: add a folder name to the browser list, formatting it as
 *   necessary. */
static void imap_add_folder (char delim, char *folder, int noselect,
                             int noinferiors, struct browser_state *state,
                             short isparent)
{
  char tmp[LONG_STRING];
  char relpath[LONG_STRING];
  IMAP_MBOX mx;

  if (imap_parse_path (state->folder, &mx))
    return;

  imap_unmunge_mbox_name (folder);

  if (state->entrylen + 1 == state->entrymax) {
    mem_realloc (&state->entry,
                  sizeof (struct folder_file) * (state->entrymax += 256));
    memset (state->entry + state->entrylen, 0,
            (sizeof (struct folder_file) *
             (state->entrymax - state->entrylen)));
  }

  /* render superiors as unix-standard ".." */
  if (isparent)
    strfcpy (relpath, "../", sizeof (relpath));
  /* strip current folder from target, to render a relative path */
  else if (!str_ncmp (mx.mbox, folder, str_len (mx.mbox)))
    strfcpy (relpath, folder + str_len (mx.mbox), sizeof (relpath));
  else
    strfcpy (relpath, folder, sizeof (relpath));

  /* apply filemask filter. This should really be done at menu setup rather
   * than at scan, since it's so expensive to scan. But that's big changes
   * to browser.c */
  if (!((regexec (Mask.rx, relpath, 0, NULL, 0) == 0) ^ Mask.not)) {
    mem_free (&mx.mbox);
    return;
  }

  imap_qualify_path (tmp, sizeof (tmp), &mx, folder);
  (state->entry)[state->entrylen].name = str_dup (tmp);

  /* mark desc with delim in browser if it can have subfolders */
  if (!isparent && !noinferiors && str_len (relpath) < sizeof (relpath) - 1) {
    relpath[str_len (relpath) + 1] = '\0';
    relpath[str_len (relpath)] = delim;
  }

  (state->entry)[state->entrylen].desc = str_dup (relpath);

  (state->entry)[state->entrylen].imap = 1;
  /* delimiter at the root is useless. */
  if (folder[0] == '\0')
    delim = '\0';
  (state->entry)[state->entrylen].delim = delim;
  (state->entry)[state->entrylen].selectable = !noselect;
  (state->entry)[state->entrylen].inferiors = !noinferiors;
  (state->entrylen)++;

  mem_free (&mx.mbox);
}

static int compare_names (struct folder_file *a, struct folder_file *b)
{
  return str_cmp (a->name, b->name);
}

static int browse_get_namespace (IMAP_DATA * idata, char *nsbuf, int nsblen,
                                 IMAP_NAMESPACE_INFO * nsi, int nsilen,
                                 int *nns)
{
  char *s;
  int n;
  char ns[LONG_STRING];
  char delim = '/';
  int type;
  int nsbused = 0;
  int rc;

  *nns = 0;
  nsbuf[nsblen - 1] = '\0';

  imap_cmd_start (idata, "NAMESPACE");

  do {
    if ((rc = imap_cmd_step (idata)) != IMAP_CMD_CONTINUE)
      break;

    s = imap_next_word (idata->cmd.buf);
    if (ascii_strncasecmp ("NAMESPACE", s, 9) == 0) {
      /* There are three sections to the response, User, Other, Shared,
       * and maybe more by extension */
      for (type = IMAP_NS_PERSONAL; *s; type++) {
        s = imap_next_word (s);
        if (*s && ascii_strncasecmp (s, "NIL", 3)) {
          s++;
          while (*s && *s != ')') {
            s++;                /* skip ( */
            /* copy namespace */
            n = 0;
            delim = '\0';

            if (*s == '\"') {
              s++;
              while (*s && *s != '\"') {
                if (*s == '\\')
                  s++;
                ns[n++] = *s;
                s++;
              }
              if (*s)
                s++;
            }
            else
              while (*s && !ISSPACE (*s)) {
                ns[n++] = *s;
                s++;
              }
            ns[n] = '\0';
            /* delim? */
            s = imap_next_word (s);
            /* delimiter is meaningless if namespace is "". Why does
             * Cyrus provide one?! */
            if (n && *s && *s == '\"') {
              if (s[1] && s[2] == '\"')
                delim = s[1];
              else if (s[1] && s[1] == '\\' && s[2] && s[3] == '\"')
                delim = s[2];
            }
            /* skip "" namespaces, they are already listed at the root */
            if ((ns[0] != '\0') && (nsbused < nsblen) && (*nns < nsilen)) {
              debug_print (3, ("adding %s\n", ns));
              nsi->type = type;
              /* Cyrus doesn't append the delimiter to the namespace,
               * but UW-IMAP does. We'll strip it here and add it back
               * as if it were a normal directory, from the browser */
              if (n && (ns[n - 1] == delim))
                ns[--n] = '\0';
              strncpy (nsbuf + nsbused, ns, nsblen - nsbused - 1);
              nsi->prefix = nsbuf + nsbused;
              nsbused += n + 1;
              nsi->delim = delim;
              nsi++;
              (*nns)++;
            }
            while (*s && *s != ')')
              s++;
            if (*s)
              s++;
          }
        }
      }
    }
  }
  while (rc == IMAP_CMD_CONTINUE);

  if (rc != IMAP_CMD_OK)
    return -1;

  return 0;
}

/* Check which namespaces have contents */
static int browse_verify_namespace (IMAP_DATA * idata,
                                    IMAP_NAMESPACE_INFO * nsi, int nns)
{
  char buf[LONG_STRING];
  int i = 0;
  char *name;
  char delim;

  for (i = 0; i < nns; i++, nsi++) {
    /* Cyrus gives back nothing if the % isn't added. This may return lots
     * of data in some cases, I guess, but I currently feel that's better
     * than invisible namespaces */
    if (nsi->delim)
      snprintf (buf, sizeof (buf), "%s \"\" \"%s%c%%\"",
                option (OPTIMAPLSUB) ? "LSUB" : "LIST", nsi->prefix,
                nsi->delim);
    else
      snprintf (buf, sizeof (buf), "%s \"\" \"%s%%\"",
                option (OPTIMAPLSUB) ? "LSUB" : "LIST", nsi->prefix);

    imap_cmd_start (idata, buf);

    nsi->listable = 0;
    nsi->home_namespace = 0;
    do {
      if (imap_parse_list_response (idata, &name, &nsi->noselect,
                                    &nsi->noinferiors, &delim) != 0)
        return -1;
      nsi->listable |= (name != NULL);
    }
    while ((ascii_strncmp (idata->cmd.buf, idata->cmd.seq, SEQLEN) != 0));
  }

  return 0;
}
