/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/mmdf_mailbox.cpp
 * @brief Implementation: MMDF Mailbox class
 */
#include <stdlib.h>

#include "mmdf_mailbox.h"

MmdfMailbox::MmdfMailbox (url_t* url_) : FileMailbox (url_) {
  this->haveFilters = 1;
}

MmdfMailbox::~MmdfMailbox (void) {
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
