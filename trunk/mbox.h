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
 * functions for dealing with mbox/mmdf style mailboxes
 */

#ifndef _MBOX_H
#define _MBOX_H

#include "mx.h"

#define MMDF_SEP "\001\001\001\001\n"

int mbox_sync_mailbox (CONTEXT *, int *);
int mbox_open_mailbox (CONTEXT *);
int mbox_check_mailbox (CONTEXT *, int *);
int mbox_close_mailbox (CONTEXT *);
int mbox_lock_mailbox (CONTEXT *, int, int);
int mbox_parse_mailbox (CONTEXT *);
int mmdf_parse_mailbox (CONTEXT *);
void mbox_unlock_mailbox (CONTEXT *);
int mbox_check_empty (const char *);

int mbox_strict_cmp_headers (const HEADER *, const HEADER *);
int mbox_reopen_mailbox (CONTEXT *, int *);

int mbox_open_new_message (MESSAGE * msg, CONTEXT * dest, HEADER * hdr);

#endif
