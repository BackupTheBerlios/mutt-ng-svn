/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-8 Michael R. Elkins <me@mutt.org>
 * Copyright (C) 2000-3 Brendan Cully <brendan@kublai.com>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#ifndef _IMAP_H
#define _IMAP_H 1

#include "account.h"
#include "browser.h"
#include "mx.h"

/* -- data structures -- */
typedef struct {
  ACCOUNT account;
  char *mbox;
} IMAP_MBOX;

/* imap.c */
int imap_access (const char *, int);
int imap_check_mailbox (CONTEXT * ctx, int *index_hint, int force);
int imap_delete_mailbox (CONTEXT * idata, IMAP_MBOX mx);
int imap_open_mailbox (CONTEXT * ctx);
int imap_open_mailbox_append (CONTEXT * ctx);
int imap_sync_mailbox (CONTEXT * ctx, int expunge, int *index_hint);
void imap_close_mailbox (CONTEXT * ctx);
int imap_buffy_check (char *path);
int imap_mailbox_check (char *path, int new);
int imap_search (CONTEXT* ctx, const pattern_t* pat);
int imap_subscribe (char *path, int subscribe);
int imap_complete (char *dest, size_t dlen, char *path);

void imap_allow_reopen (CONTEXT * ctx);
void imap_disallow_reopen (CONTEXT * ctx);

/* browse.c */
int imap_browse (char *path, struct browser_state *state);
int imap_mailbox_create (const char *folder);
int imap_mailbox_rename (const char *mailbox);

/* message.c */
int imap_append_message (CONTEXT * ctx, MESSAGE * msg);
int imap_copy_messages (CONTEXT * ctx, HEADER * h, char *dest, int delete);
int imap_fetch_message (MESSAGE * msg, CONTEXT * ctx, int msgno);

/* socket.c */
void imap_logout_all (void);

/* util.c */
int imap_expand_path (char *path, size_t len);
int imap_parse_path (const char *path, IMAP_MBOX * mx);
void imap_pretty_mailbox (char *path);

int imap_wait_keepalive (pid_t pid);
void imap_keepalive (void);

#endif
