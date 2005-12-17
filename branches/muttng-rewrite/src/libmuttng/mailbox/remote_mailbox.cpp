/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/remote_mailbox.cpp
 * @brief Implementation: Remote Mailbox base class
 */
#include <stdlib.h>
#include <iostream>

#include "remote_mailbox.h"
#include "pop3_mailbox.h"
#include "nntp_mailbox.h"

RemoteMailbox::RemoteMailbox (url_t* url_, Connection * c) : Mailbox (url_), conn(c) {
  this->haveFilters = 0;
}

RemoteMailbox::~RemoteMailbox (void) {}

Mailbox* RemoteMailbox::fromURL (url_t* url_, buffer_t* error) {
  Connection* conn = NULL;
  Mailbox* ret = NULL;

  /* first, depending on mailbox type, see if it has a connection already */
  switch(url_->proto) {
  case P_NNTP: conn = NNTPMailbox::findConnection(url_); break;
  default: break;
  }

  /* if not, create new one */
  if (!conn && !(conn = Connection::fromURL(url_,error)))
    return NULL;

  /* finally, create mailbox */
  switch (url_->proto) {
  case P_POP3: ret = new POP3Mailbox(url_,conn); break;
  case P_NNTP: ret = new NNTPMailbox(url_,conn); break;
  default:
    break;
  }
  return ret;
}
