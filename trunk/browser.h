/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#ifndef _BROWSER_H
#define _BROWSER_H 1

#ifdef USE_NNTP
#include "nntp/nntp.h"
#endif

struct folder_file {
  mode_t mode;
  off_t size;
  time_t mtime;
  struct stat *st;

  char *name;
  char *desc;

  unsigned short new;
#ifdef USE_IMAP
  char delim;

  unsigned imap:1;
  unsigned selectable:1;
  unsigned inferiors:1;
#endif
#ifdef USE_NNTP
  NNTP_DATA *nd;
#endif
  unsigned tagged:1;
};

struct browser_state {
  struct folder_file *entry;
  unsigned int entrylen;        /* number of real entries */
  unsigned int entrymax;        /* max entry */
#ifdef USE_IMAP
  short imap_browse;
  char *folder;
  unsigned noselect:1;
  unsigned marked:1;
  unsigned unmarked:1;
#endif
};

#endif /* _BROWSER_H */
