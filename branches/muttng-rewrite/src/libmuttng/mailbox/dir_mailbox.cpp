/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/dir_mailbox.cpp
 * @brief Implementation: Directory Mailbox base class
 */
#include <stdlib.h>

#include "dir_mailbox.h"

DirMailbox::DirMailbox (url_t* url_) : LocalMailbox (url_) {
  this->haveFilters = 1;
  this->cache = Cache::create();
  connectSignal (this->cache->cacheGetKey, this, &DirMailbox::cacheGetKey);
}

DirMailbox::~DirMailbox (void) {
  disconnectSignals (this->cache->cacheGetKey, this);
  delete this->cache;
}

mailbox_query_status DirMailbox::openMailbox() {
  return MQ_ERR;
}

bool DirMailbox::checkEmpty() {
  return false;
}

bool DirMailbox::checkACL(acl_bit_t bit) {
  (void)bit;
  return false;
}

mailbox_query_status DirMailbox::closeMailbox() {
  return MQ_ERR;
}

mailbox_query_status DirMailbox::syncMailbox() {
  return MQ_ERR;
}

mailbox_query_status DirMailbox::checkMailbox() {
  return MQ_ERR;
}

mailbox_query_status DirMailbox::commitMessage(Message * msg) {
  (void)msg;
  return MQ_ERR;
}

mailbox_query_status DirMailbox::openNewMessage(Message * msg) {
  (void)msg;
  return MQ_ERR;
}

bool DirMailbox::checkAccess() {
  return false;
}

mailbox_query_status DirMailbox::fetchMessageHeaders(Message * msg, unsigned int count) {
  (void)msg;
  (void)count;
  return MQ_ERR;
}

mailbox_query_status DirMailbox::fetchMessage(Message * msg, unsigned int msgnum) {
  (void)msg;
  (void)msgnum;
  return MQ_ERR;
}
