/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/mailbox.cpp
 * @brief Implementation: Mailbox bass class
 */
#include <stdlib.h>

#include "core/str.h"

#include "transport/connection.h"

#include "mailbox.h"
#include "remote_mailbox.h"
#include "local_mailbox.h"

Mailbox::Mailbox (url_t* url_) {
  this->haveCaching = 0;
  this->haveAuthentication = 0;
  this->haveEncryption = 0;
  this->url = url_;
}

Mailbox::~Mailbox (void) {
  url_free (this->url);
  delete (this->url);
}

void Mailbox::getUrl (buffer_t* dst) {
  if (dst)
    url_to_string (this->url, dst, false);
}

Mailbox* Mailbox::fromURL (const char* url_, buffer_t* error) {
  Mailbox* ret = NULL;
  url_t* u = NULL;

  if (!(u = url_from_string (url_, error)))
    return (NULL);

  switch (u->proto) {
  case P_IMAP:
  case P_POP3:
  case P_NNTP:
    ret = RemoteMailbox::fromURL(u);
    break;
  case P_FILE:
    ret = LocalMailbox::fromURL(u);
    break;
  default:
    break;
  }
  return (ret);
}
