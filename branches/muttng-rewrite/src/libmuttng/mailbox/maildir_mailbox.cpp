/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/maildir_mailbox.cpp
 * @brief Implementation: Maildir Mailbox base class
 */
#include "maildir_mailbox.h"

MaildirMailbox::MaildirMailbox(url_t* url_) : DirMailbox(url_) {}

MaildirMailbox::~MaildirMailbox() {}

bool MaildirMailbox::cacheGetKey (Message* msg, buffer_t* dst) {
  (void)msg;
  (void)dst;
  return false;
}

bool MaildirMailbox::isMaildir (url_t* url_) {
  (void)url_;
  return false;
}
