/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/local_mailbox.cpp
 * @brief Implementation: Local Mailbox base class
 */
#include <stdlib.h>

#include "local_mailbox.h"

LocalMailbox::LocalMailbox (void) {
  this->haveFilters = 1;
}

LocalMailbox::~LocalMailbox (void) {
}
