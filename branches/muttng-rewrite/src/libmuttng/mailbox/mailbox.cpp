/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/mailbox.cpp
 * @brief Implementation: Mailbox bass class
 */
#include <stdlib.h>
#include <iostream>

#include "core/str.h"
#include "core/intl.h"

#include "transport/connection.h"

#include "mailbox.h"
#include "remote_mailbox.h"
#include "local_mailbox.h"

Mailbox::Mailbox (url_t* url_) {
  this->haveCaching = 0;
  this->haveAuthentication = 0;
  this->haveEncryption = 0;
  this->url = url_;
  buffer_init(&errorMsg);
}

Mailbox::~Mailbox (void) {
  url_free (this->url);
  delete (this->url);
  buffer_free(&errorMsg);
}

void Mailbox::getURL (buffer_t* dst) {
  if (dst)
    url_to_string (this->url, dst, false);
}

Mailbox* Mailbox::fromURL (const char* url_, buffer_t* error) {
  Mailbox* ret = NULL;
  url_t* u = NULL;

  if (!(u = url_from_string (url_, error)))
    return (NULL);

  switch (u->proto) {
#if defined(LIBMUTTNG_IMAP) || defined(LIBMUTTNG_POP) || defined(LIBMUTTNG_NNTP)
#ifdef LIBMUTTNG_IMAP
  case P_IMAP:
#endif
#ifdef LIBMUTTNG_POP3
  case P_POP3:
#endif
#ifdef LIBMUTTNG_NNTP
  case P_NNTP:
#endif
    ret = RemoteMailbox::fromURL(u,error);
    break;
#endif
  case P_FILE:
    ret = LocalMailbox::fromURL(u,error);
    break;
  default:
    break;
  }
  return (ret);
}

void Mailbox::strerror (mailbox_query_status state, buffer_t* error) {
  if (!error) return;
  const char* msg = NULL;
  bool err = true;
  switch(state) {
  case MQ_ERR: msg = ("general error"); break;
  case MQ_AUTH: msg = ("not authenticated"); break;
  case MQ_NOT_CONNECTED: msg = ("not connected"); break;
  case MQ_OK: msg = ("ok"); err = false; break;
  case MQ_NEW_MAIL: msg = ("new mail"); err = false; break;
  }
  if (err && errorMsg.len) {
    buffer_add_buffer(error,&errorMsg);
    buffer_add_str(error,": ",2);
  }
  buffer_add_str(error,msg,-1);
}
