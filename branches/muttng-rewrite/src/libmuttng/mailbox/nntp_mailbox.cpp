/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/nntp_mailbox.cpp
 * @brief Implementation: NNTP Mailbox
 */
#include <stdlib.h>

#include "nntp_mailbox.h"

NNTPMailbox::NNTPMailbox (void) {
  this->haveCaching = 1;
  this->haveAuthentication = 1;
  this->haveEncryption = 1;
}

NNTPMailbox::~NNTPMailbox (void) {
}
