/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/local_mailbox.cpp
 * @brief Implementation: Local Mailbox base class
 */
#include <stdlib.h>

#include "local_mailbox.h"

LocalMailbox::LocalMailbox (url_t* url_) : Mailbox (url_) {
  this->haveFilters = 1;
}

LocalMailbox::~LocalMailbox (void) {
}
