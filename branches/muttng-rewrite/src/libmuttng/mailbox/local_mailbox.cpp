/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/local_mailbox.cpp
 * @brief Implementation: Local Mailbox base class
 */
#include <stdlib.h>

#include "local_mailbox.h"

#include "mbox_mailbox.h"
#include "mmdf_mailbox.h"
#include "maildir_mailbox.h"
#include "mh_mailbox.h"

LocalMailbox::LocalMailbox (url_t* url_) : Mailbox (url_) {
  this->haveFilters = 1;
}

LocalMailbox::~LocalMailbox (void) {
}

Mailbox* LocalMailbox::fromURL (url_t* url_) {
  if (url_->proto != P_FILE)
    return NULL;

  if (MboxMailbox::isMbox (url_))
    return new MboxMailbox(url_);
  if (MmdfMailbox::isMmdf (url_))
    return new MmdfMailbox(url_);
  if (MaildirMailbox::isMaildir(url_))
    return new MaildirMailbox(url_);
  if (MHMailbox::isMH(url_))
    return new MHMailbox(url_);

  return NULL;
}
