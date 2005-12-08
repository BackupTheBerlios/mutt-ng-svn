/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/mh_mailbox.cpp
 * @brief Implementation: MH Mailbox base class
 */
#include "mh_mailbox.h"

MHMailbox::MHMailbox(url_t* url_) : DirMailbox(url_) {}

MHMailbox::~MHMailbox() {}

bool MHMailbox::cacheGetKey (Message* msg, buffer_t* dst) {
  (void)msg;
  (void)dst;
  return false;
}

bool MHMailbox::isMH (url_t* url_, buffer_t* error) {
  (void)url_;
  (void) error;
  return false;
}
