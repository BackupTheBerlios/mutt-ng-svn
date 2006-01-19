/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/mmdf_mailbox.cpp
 * @brief Implementation: MMDF Mailbox class
 *
 * This file is published under the GNU General Public License.
 */
#include <stdlib.h>
#include "libmuttng/config/config_manager.h"
#include "mmdf_mailbox.h"

MmdfMailbox::MmdfMailbox (url_t* url_) : FileMailbox (url_) {
  this->haveFilters = 1;
}

MmdfMailbox::~MmdfMailbox (void) {
}

void MmdfMailbox::reg() {
  ConfigManager::regFeature("mmdf");
}

bool MmdfMailbox::isSeparator() {
  return false;
}

void MmdfMailbox::getSeparator(Message* msg) {
  (void)msg;
}

bool MmdfMailbox::isMmdf (buffer_t* path, struct stat* st) {
  (void) path;
  (void) st;
  return false;
}

unsigned long MmdfMailbox::msgNew() { return 0; }
unsigned long MmdfMailbox::msgOld() { return 0; }
unsigned long MmdfMailbox::msgTotal() { return 0; }
unsigned long MmdfMailbox::msgFlagged() { return 0; }
