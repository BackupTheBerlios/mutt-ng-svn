/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/pop3_mailbox.cpp
 * @brief Implementation: POP3 Mailbox
 */
#include <stdlib.h>

#include "util/url.h"
#include "pop3_mailbox.h"

POP3Mailbox::POP3Mailbox (void) {
  this->haveCaching = 1;
  this->haveAuthentication = 1;
  this->haveEncryption = 1;
}

POP3Mailbox::~POP3Mailbox (void) {
}

const char* POP3Mailbox::cacheKey (Message* msg) {
  (void) msg;
  return (NULL);
}

mailbox_query_status POP3Mailbox::openMailbox() {
	/* TODO */

	if (!url) {
		/* TODO: emit some error? */
		return MQ_ERR;
	}

	buffer_t err;
	url_t * connect_url = url_from_string(url,&err);

	if (!connect_url) {
		/* TODO: emit error message */
		return MQ_ERR;
	}

	delete connect_url;

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
