/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/dir_mailbox.cpp
 * @brief Implementation: Directory Mailbox base class
 *
 * This file is published under the GNU General Public License.
 */
#include <stdlib.h>

#include "dir_mailbox.h"

DirMailbox::DirMailbox (url_t* url_) : LocalMailbox (url_) {
  this->haveFilters = 1;
#ifdef WANT_CACHE
  this->cache = Cache::create();
  connectSignal (this->cache->cacheGetKey, this, &DirMailbox::cacheGetKey);
#endif
}

DirMailbox::~DirMailbox (void) {
#ifdef WANT_CACHE
  disconnectSignals (this->cache->cacheGetKey, this);
  delete this->cache;
#endif
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
