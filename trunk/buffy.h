/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */
#ifndef _BUFFY_H
#define _BUFFY_H

#include "lib/list.h"

/*parameter to mutt_parse_mailboxes*/
#define M_MAILBOXES   1
#define M_UNMAILBOXES 2

typedef struct buffy_t {
  char *path;
#ifdef BUFFY_SIZE
  long size;
#endif                          /* BUFFY_SIZE */
  short new;                    /* mailbox has new mail */
  short has_new;                /* set it new if new and not read */
  int msgcount;                 /* total number of messages */
  int msg_unread;               /* number of unread messages */
  int msg_flagged;              /* number of flagged messages */
  short notified;               /* user has been notified */
  short magic;                  /* mailbox type */
  short newly_created;          /* mbox or mmdf just popped into existence */
} BUFFY;

/* folders with incomming mail (via mailboxes command) */
WHERE list2_t* Incoming;
WHERE short BuffyTimeout INITVAL (3);
extern time_t BuffyDoneTime;    /* last time we knew for sure how much mail there was */

/* looks up a path in Incoming list (returns index) */
int buffy_lookup (const char*);
/* handles mailboxes commands */
int buffy_parse_mailboxes (BUFFER*, BUFFER*, unsigned long, BUFFER*);
/* from given path, gets next mailbox in Incoming with new mail */
void buffy_next (char*, size_t);
/* checks mailboxes for new mail (returns number) */
int buffy_check (int);
/* lists mailboxes with new mail */
int buffy_list (void);
/* wrapper around buffy_list() */
int buffy_notify (void);

#ifdef BUFFY_SIZE
BUFFY *buffy_find_mailbox (const char *path);
void buffy_update_mailbox (BUFFY * b);
#endif

#endif /* !_BUFFY_H */
