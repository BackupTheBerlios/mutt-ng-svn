/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2002 Michael R. Elkins <me@mutt.org>
 * Copyright (C) 1999-2002 Thomas Roessler <roessler@does-not-exist.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

/*
 * functions for dealing with Maildir/MH style mailboxes
 */

#ifndef _MH_H
#define _MH_H

#include "mx.h"

int mh_read_dir (CONTEXT *);
int mh_sync_mailbox (CONTEXT *, int *);
int mh_check_mailbox (CONTEXT *, int *);
int mh_buffy (const char *);
int mh_check_empty (const char *);
int mh_commit_message (CONTEXT *, MESSAGE *, HEADER *);
int mh_open_new_message (MESSAGE *, CONTEXT *, HEADER *);
int mh_is_magic (const char*);

int maildir_read_dir (CONTEXT *);
int maildir_check_mailbox (CONTEXT *, int *);
int maildir_check_empty (const char *);
int maildir_commit_message (CONTEXT *, MESSAGE *, HEADER *);
int maildir_open_new_message (MESSAGE *, CONTEXT *, HEADER *);
FILE *maildir_open_find_message (const char *, const char *);
int maildir_is_magic (const char*);

mx_t* maildir_reg_mx (void);
mx_t* mh_reg_mx (void);

#endif /* !_MH_H */
