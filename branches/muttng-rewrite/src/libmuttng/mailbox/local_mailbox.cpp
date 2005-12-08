/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/local_mailbox.cpp
 * @brief Implementation: Local Mailbox base class
 */
#include <stdlib.h>

#include "core/intl.h"

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

Mailbox* LocalMailbox::fromURL (url_t* url_, buffer_t* error) {
  if (url_->proto != P_FILE)
    return NULL;

  if (MboxMailbox::isMbox (url_,error))
    return new MboxMailbox(url_);
  if (MmdfMailbox::isMmdf (url_,error))
    return new MmdfMailbox(url_);
  if (MaildirMailbox::isMaildir(url_,error))
    return new MaildirMailbox(url_);
  if (MHMailbox::isMH(url_,error))
    return new MHMailbox(url_);

  if (error) {
    buffer_add_str(error,_("unsupported mailbox format in '"),-1);
    buffer_add_str(error,url_->path,-1);
    buffer_add_ch(error,'\'');
  }

  return NULL;
}
