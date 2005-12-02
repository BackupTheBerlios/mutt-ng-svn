/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/pop3_mailbox.cpp
 * @brief Implementation: POP3 Mailbox
 */
#include <stdlib.h>

#include "util/url.h"
#include "pop3_mailbox.h"

POP3Mailbox::POP3Mailbox (url_t* url_, Connection * c) : RemoteMailbox (url_,c) {
  this->haveCaching = 1;
  this->haveAuthentication = 1;
  this->haveEncryption = 1;
  this->cache = Cache::create ();
  connectSignal (this->cache->cacheGetKey, this, &POP3Mailbox::cacheGetKey);
}

POP3Mailbox::~POP3Mailbox (void) {
  disconnectSignals (this->cache->cacheGetKey, this);
  delete this->cache;
}

mailbox_query_status POP3Mailbox::openMailbox() {
  /* TODO */

  if (!url) {
    /* TODO: emit some error? */
    return MQ_ERR;
  }

  return MQ_NOT_CONNECTED;
}

bool POP3Mailbox::checkEmpty() {
  /* TODO */
  return MQ_NOT_CONNECTED;
}

bool POP3Mailbox::checkACL(acl_bit_t bit) {
  switch (bit) {
  case ACL_INSERT:    /* editing messages */
  case ACL_WRITE:     /* change importance */
    return false;
  case ACL_DELETE:    /* (un)deletion */
  case ACL_SEEN:      /* mark as read */
    return true;
  default:
    return false;
  }
}

mailbox_query_status POP3Mailbox::closeMailbox() {
  return MQ_NOT_CONNECTED;
}

mailbox_query_status POP3Mailbox::syncMailbox() {
  return MQ_NOT_CONNECTED;
}

mailbox_query_status POP3Mailbox::checkMailbox() {
  return MQ_NOT_CONNECTED;
}

mailbox_query_status POP3Mailbox::commitMessage(Message * msg) {
  (void)msg;
  return MQ_ERR;
}

mailbox_query_status POP3Mailbox::openNewMessage(Message * msg) {
  (void)msg;
  return MQ_ERR;
}

bool POP3Mailbox::checkAccess() {
  return false;
}

mailbox_query_status POP3Mailbox::fetchMessageHeaders(Message * msg, unsigned int msgnum) {
  (void)msg;
  (void)msgnum;

  return MQ_NOT_CONNECTED;
}

mailbox_query_status POP3Mailbox::fetchMessage(Message * msg, unsigned int msgnum) {
  (void)msg;
  (void)msgnum;

  return MQ_NOT_CONNECTED;
}

bool POP3Mailbox::cacheGetKey (Message* msg, buffer_t* dst) {
  (void) msg;
  (void) dst;
  return (false);
}
