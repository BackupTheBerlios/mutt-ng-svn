/*
 * Copyright notice from original mutt:
 * Copyright (C) 2000-5 Brendan Cully <brendan@kublai.com>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

/* common SASL helper routines */

#ifndef _MUTT_SASL_H_
#define _MUTT_SASL_H_ 1

#ifdef USE_SASL2
#include <sasl/sasl.h>
#else
#include <sasl.h>
#endif

#include "mutt_socket.h"

int mutt_sasl_client_new (CONNECTION *, sasl_conn_t **);
sasl_callback_t *mutt_sasl_get_callbacks (ACCOUNT *);
int mutt_sasl_interact (sasl_interact_t *);
void mutt_sasl_setup_conn (CONNECTION *, sasl_conn_t *);

typedef struct {
  sasl_conn_t *saslconn;
  const sasl_ssf_t *ssf;
  const unsigned int *pbufsize;

  /* read buffer */
#ifdef USE_SASL2
  const char *buf;
#else
  char *buf;
#endif
  unsigned int blen;
  unsigned int bpos;

  /* underlying socket data */
  void *sockdata;
  int (*msasl_open) (CONNECTION * conn);
  int (*msasl_close) (CONNECTION * conn);
  int (*msasl_read) (CONNECTION * conn, char *buf, size_t len);
  int (*msasl_write) (CONNECTION * conn, const char *buf, size_t count);
} SASL_DATA;

#endif /* _MUTT_SASL_H_ */
