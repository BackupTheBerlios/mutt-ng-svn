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
 * mailbox abstraction
 * when adding code dealing with folders or whatever,
 * please use these only
 */

#ifndef _MX_H
#define _MX_H

#include <sys/stat.h>
#include <utime.h>

/*
 * supported mailbox formats
 * in mx_init() the registration order must be exactly as given here!!!1!
 */
enum {
  M_MBOX = 1,
  M_MMDF,
  M_MH,
  M_MAILDIR
#ifdef USE_IMAP
    , M_IMAP
#endif
#ifdef USE_POP
    , M_POP
#endif
#ifdef USE_NNTP
    , M_NNTP
#endif
#ifdef USE_COMPRESSED
    , M_COMPRESSED
#endif
};

enum {
  ACL_LOOKUP = 0,
  ACL_READ,
  ACL_SEEN,
  ACL_WRITE,
  ACL_INSERT,
  ACL_POST,
  ACL_CREATE,
  ACL_DELETE,
  ACL_ADMIN,

  RIGHTSMAX
};

/* ugly hack to define macro once (for pager+index) */
#define CHECK_MX_ACL(c,f,s) if(!mx_acl_check(c,f)) \
                     {\
                        mutt_flushinp (); \
                        mutt_error(_("%s not permitted by ACL."), s); \
                        break; \
                     }

typedef struct {
  FILE *fp;                     /* pointer to the message data */
  char *path;                   /* path to temp file */
  short magic;                  /* type of mailbox this message belongs to */
  short write;                  /* nonzero if message is open for writing */
  struct {
    unsigned read:1;
    unsigned flagged:1;
    unsigned replied:1;
  } flags;
  time_t received;              /* the time at which this message was received */
} MESSAGE;

typedef struct {
  /* folder magic */
  int type;
  /* may we stat() it? */
  unsigned int local : 1;
  /* tests if given path is of its magic */
  int (*mx_is_magic) (const char*, struct stat*);
  /* tests if folder is empty */
  int (*mx_check_empty) (const char*);
  /* test for access */
  int (*mx_access) (const char*, int);
  /* read mailbox into ctx structure */
  int (*mx_open_mailbox) (CONTEXT*);
  /* open new message */
  int (*mx_open_new_message) (MESSAGE*, CONTEXT*, HEADER*);
  /* check ACL flags; if not implemented, always assume granted
   * permissions */
  int (*mx_acl_check) (CONTEXT*, int);
  /* check for new mail; see mx_check_mailbox() below for return vals */
  int (*mx_check_mailbox) (CONTEXT*, int*, int);
  /* fast closing */
  void (*mx_fastclose_mailbox) (CONTEXT*);
  /* write out changes */
  int (*mx_sync_mailbox) (CONTEXT*, int, int*);
} mx_t;

/* called from main: init all folder types */
void mx_init (void);

/* flags for mx_open_mailbox() */
#define M_NOSORT        (1<<0)  /* do not sort the mailbox after opening it */
#define M_APPEND        (1<<1)  /* open mailbox for appending messages */
#define M_READONLY      (1<<2)  /* open in read-only mode */
#define M_QUIET         (1<<3)  /* do not print any messages */
#define M_NEWFOLDER     (1<<4)  /* create a new folder - same as M_APPEND, but uses
                                 * safe_fopen() for mbox-style folders. */
#define M_COUNT         (1<<5)  /* just do counting? needed to do speed optimizations
                                   for sidebar */

/* mx_open_new_message() */
#define M_ADD_FROM      1       /* add a From_ line */

#define MAXLOCKATTEMPT 5

WHERE short DefaultMagic INITVAL (M_MBOX);

/*
 * please use the following _ONLY_ when doing "something"
 * with folders
 */

CONTEXT *mx_open_mailbox (const char *, int, CONTEXT *);

MESSAGE *mx_open_message (CONTEXT *, int);
MESSAGE *mx_open_new_message (CONTEXT *, HEADER *, int);

void mx_fastclose_mailbox (CONTEXT *);

int mx_close_mailbox (CONTEXT *, int *);
int mx_sync_mailbox (CONTEXT *, int *);
int mx_commit_message (MESSAGE *, CONTEXT *);
int mx_close_message (MESSAGE **);

/* determines magic for given folder */
int mx_get_magic (const char*);
/* sets/parses DefaultMagic */
int mx_set_magic (const char *);
/* tests whether given folder magic is (valid and) local */
int mx_is_local (int);

/* return values from mx_check_mailbox() */
enum {
  M_NEW_MAIL = 1,               /* new mail received in mailbox */
  M_LOCKED,                     /* couldn't lock the mailbox */
  M_REOPENED,                   /* mailbox was reopened */
  M_FLAGS                       /* nondestructive flags change (IMAP) */
};
int mx_check_mailbox (CONTEXT *, int *, int);

int mx_access (const char *, int);
int mx_check_empty (const char *);

int mx_acl_check (CONTEXT*, int);

void mx_alloc_memory (CONTEXT *);
void mx_update_context (CONTEXT *, int);
void mx_update_tables (CONTEXT *, int);

int mx_lock_file (const char *, int, int, int, int);
int mx_unlock_file (const char *path, int fd, int dot);

#endif /* !_MX_H */
