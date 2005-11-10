/** @ingroup libmuttng_mailbox */
/**
 * @file local_mailbox.cpp
 * @author Someone
 * @version $Id$
 * @brief Local Mailbox implementation.
 */
#include <stdlib.h>

#include "local_mailbox.h"

LocalMailbox::LocalMailbox (void) {
  this->haveFilters = 1;
}

LocalMailbox::~LocalMailbox (void) {
}
