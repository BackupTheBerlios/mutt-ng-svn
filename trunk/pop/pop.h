/*
 * Copyright notice from original mutt:
 * Copyright (C) 2000-2003 Vsevolod Volkov <vvv@mutt.org.ua>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#ifndef _POP_H
#define _POP_H 1

#include "mailbox.h"
#include "mutt_socket.h"

#define POP_PORT 110
#define POP_SSL_PORT 995

/* number of entries in the hash table */
#define POP_CACHE_LEN 10

/* maximal length of the server response (RFC1939) */
#define POP_CMD_RESPONSE 512

enum {
  /* Status */
  POP_NONE = 0,
  POP_CONNECTED,
  POP_DISCONNECTED,
  POP_BYE
};

typedef enum {
  POP_A_SUCCESS = 0,
  POP_A_SOCKET,
  POP_A_FAILURE,
  POP_A_UNAVAIL
} pop_auth_res_t;

typedef struct {
  unsigned int index;
  char *path;
} POP_CACHE;

typedef enum pop_query_status_e {
  PFD_FUNCT_ERROR = -3, /* pop_fetch_data uses pop_query_status and this return value */
  PQ_ERR = -2,
  PQ_NOT_CONNECTED = -1,
  PQ_OK = 0
} pop_query_status;

typedef enum cmd_status_e {
  CMD_NOT_AVAILABLE = 0,
  CMD_AVAILABLE,
  CMD_UNKNOWN /* unknown whether it is available or not */
} cmd_status;

typedef struct {
  CONNECTION *conn;
  unsigned int status:2;
  unsigned int capabilities:1;
  unsigned int use_stls:2;
  cmd_status cmd_capa;      /* optional command CAPA */
  cmd_status cmd_stls;      /* optional command STLS */
  cmd_status cmd_user;      /* optional command USER */
  cmd_status cmd_uidl;      /* optional command UIDL */
  cmd_status cmd_top;       /* optional command TOP */
  unsigned int resp_codes:1;    /* server supports extended response codes */
  unsigned int expire:1;        /* expire is greater than 0 */
  unsigned int clear_cache:1;
  size_t size;
  time_t check_time;
  time_t login_delay;           /* minimal login delay  capability */
  char *auth_list;              /* list of auth mechanisms */
  char *timestamp;
  char err_msg[POP_CMD_RESPONSE];
  POP_CACHE cache[POP_CACHE_LEN];
} POP_DATA;

typedef struct {
  /* do authentication, using named method or any available if method is NULL */
  pop_auth_res_t (*authenticate) (POP_DATA *, const char *);
  /* name of authentication method supported, NULL means variable. If this
   * is not null, authenticate may ignore the second parameter. */
  const char *method;
} pop_auth_t;

/* pop_auth.c */
int pop_authenticate (POP_DATA *);
void pop_apop_timestamp (POP_DATA *, char *);

/* pop_lib.c */
#define pop_query(A,B,C) pop_query_d(A,B,C,NULL)
int pop_parse_path (const char *, ACCOUNT *);
int pop_connect (POP_DATA *);
pop_query_status pop_open_connection (POP_DATA *);
pop_query_status pop_query_d (POP_DATA *, char *, size_t, char *);
pop_query_status pop_fetch_data (POP_DATA *, char *, char *, int (*funct) (char *, void *),
                    void *);
pop_query_status pop_reconnect (CONTEXT *);
void pop_logout (CONTEXT *);
void pop_error (POP_DATA *, char *);

/* pop.c */
int pop_check_mailbox (CONTEXT *, int *);
int pop_open_mailbox (CONTEXT *);
pop_query_status pop_sync_mailbox (CONTEXT *, int *);
int pop_fetch_message (MESSAGE *, CONTEXT *, int);
void pop_close_mailbox (CONTEXT *);
void pop_fetch_mail (void);

#endif
