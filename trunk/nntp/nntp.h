/*
 * Copyright notice from original mutt:
 * Copyright (C) 1998 Brandon Long <blong@fiction.net>
 * Copyright (C) 1999 Andrej Gritsenko <andrej@lucky.net>
 * Copyright (C) 2000-2002 Vsevolod Volkov <vvv@mutt.org.ua>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#ifndef _NNTP_H_
#define _NNTP_H_ 1

#include "mutt_socket.h"
#include "mx.h"

#include <time.h>

#define NNTP_PORT 119
#define NNTP_SSL_PORT 563

/* number of entries in the hash table */
#define NNTP_CACHE_LEN 10

enum {
  NNTP_NONE = 0,
  NNTP_OK,
  NNTP_BYE
};

typedef struct {
  int first;
  int last;
} NEWSRC_ENTRY;

typedef struct {
  unsigned int hasXPAT:1;
  unsigned int hasXGTITLE:1;
  unsigned int hasXOVER:1;
  unsigned int hasLISTGROUP:1;
  unsigned int status:3;
  char *newsrc;
  char *cache;
  int stat;
  off_t size;
  time_t mtime;
  time_t newgroups_time;
  time_t check_time;
  HASH *newsgroups;
  LIST *list;                   /* list of newsgroups */
  LIST *tail;                   /* last entry of list */
  CONNECTION *conn;
} NNTP_SERVER;

typedef struct {
  unsigned int index;
  char *path;
} NNTP_CACHE;

typedef struct {
  NEWSRC_ENTRY *entries;
  unsigned int num;             /* number of used entries */
  unsigned int max;             /* number of allocated entries */
  unsigned int unread;
  unsigned int firstMessage;
  unsigned int lastMessage;
  unsigned int lastLoaded;
  unsigned int lastCached;
  unsigned int subscribed:1;
  unsigned int rc:1;
  unsigned int new:1;
  unsigned int allowed:1;
  unsigned int deleted:1;
  char *group;
  char *desc;
  char *cache;
  NNTP_SERVER *nserv;
  NNTP_CACHE acache[NNTP_CACHE_LEN];
} NNTP_DATA;

/* internal functions */
int nntp_get_active (NNTP_SERVER *);
int nntp_get_cache_all (NNTP_SERVER *);
int nntp_save_cache_index (NNTP_SERVER *);
int nntp_check_newgroups (NNTP_SERVER *, int);
int nntp_save_cache_group (CONTEXT *);
int nntp_parse_url (const char *, ACCOUNT *, char *, size_t);
void newsrc_gen_entries (CONTEXT *);
void nntp_get_status (CONTEXT *, HEADER *, char *, int);
void mutt_newsgroup_stat (NNTP_DATA *);
void nntp_delete_cache (NNTP_DATA *);
void nntp_add_to_list (NNTP_SERVER *, NNTP_DATA *);
void nntp_cache_expand (char *, const char *);
void nntp_delete_data (void *);

/* exposed interface */
NNTP_SERVER *mutt_select_newsserver (char *);
NNTP_DATA *mutt_newsgroup_subscribe (NNTP_SERVER *, char *);
NNTP_DATA *mutt_newsgroup_unsubscribe (NNTP_SERVER *, char *);
NNTP_DATA *mutt_newsgroup_catchup (NNTP_SERVER *, char *);
NNTP_DATA *mutt_newsgroup_uncatchup (NNTP_SERVER *, char *);
void nntp_clear_cacheindex (NNTP_SERVER *);
int mutt_newsrc_update (NNTP_SERVER *);
int nntp_open_mailbox (CONTEXT *);
int nntp_sync_mailbox (CONTEXT *);
int nntp_check_mailbox (CONTEXT *);
int nntp_close_mailbox (CONTEXT *);
void nntp_fastclose_mailbox (CONTEXT *);
int nntp_fetch_message (MESSAGE *, CONTEXT *, int);
int nntp_post (const char *);
int nntp_check_msgid (CONTEXT *, const char *);
int nntp_check_children (CONTEXT *, const char *);
void nntp_buffy (char *);
void nntp_expand_path (char *, size_t, ACCOUNT *);
void nntp_logout_all ();
const char *nntp_format_str (char *, size_t, char, const char *, const char *,
                             const char *, const char *, unsigned long,
                             format_flag);

NNTP_SERVER *CurrentNewsSrv INITVAL (NULL);

#endif /* _NNTP_H_ */
