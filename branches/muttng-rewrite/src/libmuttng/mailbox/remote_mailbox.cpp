/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/remote_mailbox.cpp
 * @brief Implementation: Remote Mailbox base class
 */
#include <stdlib.h>

#include "remote_mailbox.h"

RemoteMailbox::RemoteMailbox (url_t* url_, Connection * c) : Mailbox (url_), conn(c) {
  this->haveFilters = 0;
}

RemoteMailbox::~RemoteMailbox (void) {
}
