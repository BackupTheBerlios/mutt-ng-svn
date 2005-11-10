/** @ingroup libmuttng_mailbox */
/**
 * @file pop3_mailbox.cpp
 * @author Someone
 * @version $Id$
 * @brief POP3 Mailbox implementation.
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
