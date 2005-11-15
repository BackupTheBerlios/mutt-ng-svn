/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/pop3_mailbox.cpp
 * @brief Implementation: POP3 Mailbox
 */
#include <stdlib.h>

#include "pop3_mailbox.h"

POP3Mailbox::POP3Mailbox (void) {
  this->haveCaching = 1;
  this->haveAuthentication = 1;
  this->haveEncryption = 1;
}

POP3Mailbox::~POP3Mailbox (void) {
}
