/** @ingroup libmuttng_mailbox */
/**
 * @file remote_mailbox.cpp
 * @author Someone
 * @version $Id$
 * @brief Remote Mailbox implementation.
 */
#include <stdlib.h>

#include "remote_mailbox.h"

RemoteMailbox::RemoteMailbox (void) {
  this->haveFilters = 0;
}

RemoteMailbox::~RemoteMailbox (void) {
}
