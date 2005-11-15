/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/imap_mailbox.cpp
 * @brief Implementation: IMAP Mailbox
 */
#include <stdlib.h>

#include "imap_mailbox.h"

ImapMailbox::ImapMailbox (void) {
  this->haveCaching = 1;
  this->haveAuthentication = 1;
  this->haveEncryption = 1;
}

ImapMailbox::~ImapMailbox (void) {
}
