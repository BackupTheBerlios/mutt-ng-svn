/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/mailbox.cpp
 * @brief Implementation: Mailbox bass class
 */
#include <stdlib.h>

#include "core/str.h"

#include "mailbox.h"

Mailbox::Mailbox (const char* url) {
  this->haveCaching = 0;
  this->haveAuthentication = 0;
  this->haveEncryption = 0;
  this->url = url;
  DEBUGPRINT(D_MOD,("create mailbox '%s'", NONULL(url)));
}

Mailbox::~Mailbox (void) {
}

const char* Mailbox::getUrl () {
  return (this->url);
}
