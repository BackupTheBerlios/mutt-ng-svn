/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/file_mailbox.cpp
 * @brief Implementation: Single file Mailbox base class
 */
#include <stdlib.h>

#include "file_mailbox.h"

FileMailbox::FileMailbox (url_t* url_) : LocalMailbox (url_) {
  this->haveFilters = 1;
}

FileMailbox::~FileMailbox (void) {}

mailbox_query_status FileMailbox::openMailbox() {
  return MQ_ERR;
}

bool FileMailbox::checkEmpty() {
  return false;
}

bool FileMailbox::checkACL(acl_bit_t bit) {
  (void)bit;
  return false;
}

mailbox_query_status FileMailbox::closeMailbox() {
  return MQ_ERR;
}

mailbox_query_status FileMailbox::syncMailbox() {
  return MQ_ERR;
}

mailbox_query_status FileMailbox::checkMailbox() {
  return MQ_ERR;
}

mailbox_query_status FileMailbox::commitMessage(Message * msg) {
  (void)msg;
  return MQ_ERR;
}

mailbox_query_status FileMailbox::openNewMessage(Message * msg) {
  (void)msg;
  return MQ_ERR;
}

bool FileMailbox::checkAccess() {
  return false;
}

mailbox_query_status FileMailbox::fetchMessageHeaders(Message * msg, unsigned int count) {
  (void)msg;
  (void)count;
  return MQ_ERR;
}

mailbox_query_status FileMailbox::fetchMessage(Message * msg, unsigned int msgnum) {
  (void)msg;
  (void)msgnum;
  return MQ_ERR;
}
