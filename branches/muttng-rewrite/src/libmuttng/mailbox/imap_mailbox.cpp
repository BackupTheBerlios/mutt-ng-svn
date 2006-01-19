/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/imap_mailbox.cpp
 * @brief Implementation: IMAP Mailbox
 *
 * This file is published under the GNU General Public License.
 */
#include <stdlib.h>
#include "libmuttng/config/config_manager.h"
#include "imap_mailbox.h"

ImapMailbox::ImapMailbox (url_t* url_) : RemoteMailbox (url_) {
  this->haveCaching = 1;
  this->haveAuthentication = 1;
  this->haveEncryption = 1;
}

ImapMailbox::~ImapMailbox (void) {
}

void ImapMailbox::reg() {
  ConfigManager::regFeature("imap");
}
