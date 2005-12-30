/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/mbox_mailbox.cpp
 * @brief Implementation: MBOX Mailbox class
 */
#include <stdlib.h>
#include "libmuttng/config/config_manager.h"
#include "mbox_mailbox.h"

MboxMailbox::MboxMailbox (url_t* url_) : FileMailbox (url_) {
  this->haveFilters = 1;
}

MboxMailbox::~MboxMailbox (void) {
}

void MboxMailbox::reg() {
  ConfigManager::regFeature("mbox");
}

bool MboxMailbox::isSeparator() {
  return false;
}

void MboxMailbox::getSeparator(Message* msg) {
  (void)msg;
}

bool MboxMailbox::isMbox (buffer_t* path, struct stat* st) {
  (void) path;
  (void) st;
  return false;
}

unsigned long MboxMailbox::msgNew() { return 0; }
unsigned long MboxMailbox::msgOld() { return 0; }
unsigned long MboxMailbox::msgTotal() { return 0; }
unsigned long MboxMailbox::msgFlagged() { return 0; }
