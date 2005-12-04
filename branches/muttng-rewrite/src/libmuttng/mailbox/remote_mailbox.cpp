/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/remote_mailbox.cpp
 * @brief Implementation: Remote Mailbox base class
 */
#include <stdlib.h>

#include "remote_mailbox.h"
#include "pop3_mailbox.h"

RemoteMailbox::RemoteMailbox (url_t* url_, Connection * c) : Mailbox (url_), conn(c) {
  this->haveFilters = 0;
}

RemoteMailbox::~RemoteMailbox (void) {}

Mailbox* RemoteMailbox::fromURL (url_t* url_) {
  Connection* conn = NULL;
  Mailbox* ret = NULL;

  if (!(conn = Connection::fromURL(url_)))
    return NULL;
  switch (url_->proto) {
  case P_POP3:
    ret = new POP3Mailbox(url_,conn);
    break;
  default:
    break;
  }
  return ret;
}
