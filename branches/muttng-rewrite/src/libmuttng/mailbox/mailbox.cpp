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

  std::cout<<u->host<<std::endl;
  std::cout<<u->port<<std::endl;
  std::cout<<u->defport<<std::endl;

  switch (u->proto) {
  case P_IMAP:
  case P_POP3:
  case P_NNTP:
    ret = RemoteMailbox::fromURL(u,error);
    break;
  case P_FILE:
    ret = LocalMailbox::fromURL(u,error);
    break;
  default:
    break;
  }
  return (ret);
}

const char* Mailbox::strerror (mailbox_query_status state) {
  switch(state) {
  case MQ_ERR: return _("general error");
  case MQ_AUTH: return _("not authenticated");
  case MQ_NOT_CONNECTED: return _("not connected");
  case MQ_OK: return _("ok");
  case MQ_NEW_MAIL: return _("new mail");
  }
  return NULL;
}
