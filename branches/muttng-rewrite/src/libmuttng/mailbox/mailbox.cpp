/** @ingroup libmuttng_mailbox */
/**
 * @file mailbox.cpp
 * @author Someone
 * @version $Id$
 * @brief Mailbox implementation.
 */
#include <stdlib.h>

#include "core/str.h"

#include "mailbox.h"

Mailbox::Mailbox (const char* url) {
  this->haveCaching = 0;
  this->haveAuthentication = 0;
  this->haveEncryption = 0;
  this->url = url;
  DEBUGPRINT(1,("create mailbox '%s'", NONULL(url)));
}

Mailbox::~Mailbox (void) {
}

const char* Mailbox::getUrl () {
  return (this->url);
}
