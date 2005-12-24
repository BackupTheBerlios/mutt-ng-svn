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

void Mailbox::getURL (buffer_t* dst) {
  if (dst)
    url_to_string (this->url, dst, false);
}

Mailbox* Mailbox::fromURL (const char* url_) {
  Mailbox* ret = NULL;
  url_t* u = NULL;
  buffer_t error;

  buffer_init(&error);
  buffer_add_str(&error,_("Error opening folder '"),-1);
  buffer_add_str(&error,url_,-1);
  buffer_add_str(&error,_("': "),-1);

  if (!(u = url_from_string (url_, &error))) {
//    displayError->emit(&error);
    buffer_free(&error);
    return (NULL);
  }

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
    ret = RemoteMailbox::fromURL(u,&error);
    break;
#endif
  case P_FILE:
    ret = LocalMailbox::fromURL(u,&error);
    break;
  default:
    buffer_add_str(&error,_("protocoll unsupported."),-1);
    // displayError->emit(&error);
    break;
  }
  buffer_free(&error);
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
