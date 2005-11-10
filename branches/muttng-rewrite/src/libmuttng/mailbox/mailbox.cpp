/** @ingroup libmuttng_mailbox */
/**
 * @file mailbox.cpp
 * @author Someone
 * @version $Id$
 * @brief Mailbox implementation.
 */
#include <stdlib.h>

#include "mailbox.h"

Mailbox::Mailbox (void) {
  this->haveCaching = 0;
  this->haveAuthentication = 0;
  this->haveEncryption = 0;
  this->url = NULL;
}

Mailbox::Mailbox (const char* url) {
  Mailbox ();
  this->url = url;
}

Mailbox::~Mailbox (void) {
}

const char* Mailbox::getUrl () {
  return (this->url);
}
