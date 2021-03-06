/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/remote_mailbox.cpp
 * @brief Implementation: Remote Mailbox base class
 *
 * This file is published under the GNU General Public License.
 */
#include <stdlib.h>
#include <iostream>

#include "remote_mailbox.h"
#include "pop3_mailbox.h"
#include "nntp_mailbox.h"

RemoteMailbox::RemoteMailbox (url_t* url_, Connection * c) : Mailbox (url_), conn(c) {
  this->haveFilters = 0;
  buffer_init(&rbuf);
  buffer_init(&sbuf);
}

RemoteMailbox::~RemoteMailbox (void) {
  buffer_free(&rbuf);
  buffer_free(&sbuf);
}

Mailbox* RemoteMailbox::fromURL (url_t* url_, buffer_t* error) {
  Connection* conn = NULL;
  Mailbox* ret = NULL;

  /* if not, create new one */
  if (!(conn = Connection::fromURL(url_,error)))
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
