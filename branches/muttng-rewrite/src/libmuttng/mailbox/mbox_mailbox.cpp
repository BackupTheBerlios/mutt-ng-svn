/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/mbox_mailbox.cpp
 * @brief Implementation: MBOX Mailbox class
 */
#include <stdlib.h>

#include "mbox_mailbox.h"

MboxMailbox::MboxMailbox (url_t* url_) : FileMailbox (url_) {
  this->haveFilters = 1;
}

MboxMailbox::~MboxMailbox (void) {
}

bool MboxMailbox::isSeparator() {
  return false;
}

void MboxMailbox::getSeparator(Message* msg) {
  (void)msg;
}

bool MboxMailbox::isMbox (url_t* url_, buffer_t* error) {
  (void)url_;
  (void) error;
  return false;
}

unsigned long MboxMailbox::msgNew() { return 0; }
unsigned long MboxMailbox::msgOld() { return 0; }
unsigned long MboxMailbox::msgTotal() { return 0; }
unsigned long MboxMailbox::msgFlagged() { return 0; }
