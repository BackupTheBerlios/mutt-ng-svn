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

bool MmdfMailbox::isMmdf (url_t* url_, buffer_t* error) {
  (void)url_;
  (void) error;
  return false;
}
