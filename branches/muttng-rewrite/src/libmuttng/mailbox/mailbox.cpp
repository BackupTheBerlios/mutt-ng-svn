/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/mailbox.cpp
 * @brief Implementation: Mailbox bass class
 */
#include <stdlib.h>

#include "core/str.h"

#include "mailbox.h"
#include "pop3_mailbox.h"
#include "transport/connection.h"

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
  Connection * conn = NULL;
  url_t* u = NULL;

  if (!(u = url_from_string (url_, error)))
    return (NULL);
  switch (u->proto) {
  case P_POP3:
    conn = Connection::fromURL(u);
    if (conn) {
      ret = new POP3Mailbox(u,conn);
    }
    break;
  default:
    break;
  }
  return (ret);
}
