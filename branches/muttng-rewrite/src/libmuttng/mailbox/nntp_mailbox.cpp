/** @ingroup libmuttng_mailbox */
/**
 * @file nntp_mailbox.cpp
 * @author Someone
 * @version $Id$
 * @brief NNTP Mailbox implementation.
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
