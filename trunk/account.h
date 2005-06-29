/*
 * Copyright notice from original mutt:
 * Copyright (C) 2000-5 Brendan Cully <brendan@kublai.com>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

/* remote host account manipulation (POP/IMAP) */

#ifndef _MUTT_ACCOUNT_H_
#define _MUTT_ACCOUNT_H_ 1

#include "url.h"

/* account types */
enum {
  M_ACCT_TYPE_NONE = 0,
  M_ACCT_TYPE_IMAP,
  M_ACCT_TYPE_NNTP,
  M_ACCT_TYPE_POP
};

/* account flags */
#define M_ACCT_PORT     (1<<0)
#define M_ACCT_USER     (1<<1)
#define M_ACCT_LOGIN    (1<<1)
#define M_ACCT_PASS     (1<<2)
#define M_ACCT_SSL      (1<<3)

typedef struct {
  char user[64];
  char login[64];
  char pass[64];
  char host[128];
  unsigned short port;
  unsigned char type;
  unsigned char flags;
} ACCOUNT;

int mutt_account_match (const ACCOUNT * a1, const ACCOUNT * m2);
int mutt_account_fromurl (ACCOUNT * account, ciss_url_t * url);
void mutt_account_tourl (ACCOUNT * account, ciss_url_t * url);
int mutt_account_getuser (ACCOUNT * account);
int mutt_account_getlogin (ACCOUNT * account);
int mutt_account_getpass (ACCOUNT * account);
void mutt_account_unsetpass (ACCOUNT * account);

#endif /* _MUTT_ACCOUNT_H_ */
