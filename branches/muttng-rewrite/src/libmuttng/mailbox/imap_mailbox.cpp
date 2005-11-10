/** @ingroup libmuttng_mailbox */
/**
 * @file imap_mailbox.cpp
 * @author Someone
 * @version $Id$
 * @brief IMAP Mailbox implementation.
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
