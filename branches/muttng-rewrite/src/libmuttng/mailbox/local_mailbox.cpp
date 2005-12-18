/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/local_mailbox.cpp
 * @brief Implementation: Local Mailbox base class
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

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

  struct stat st;
  if (stat(url_->path,&st)==-1) {
    if (error) {
      buffer_add_str(error,_("Failed to detect type of folder '"),-1);
      buffer_add_str(error,url_->path,-1);
      buffer_add_str(error,_("': "),-1);
      buffer_add_str(error,::strerror(errno),-1);
    }
    return NULL;
  }

  Mailbox* ret = NULL;
  buffer_t path;
  buffer_init(&path);
  buffer_add_str(&path,url_->path,-1);

  if (MboxMailbox::isMbox (&path,&st))
    ret = new MboxMailbox(url_);
  if (MmdfMailbox::isMmdf (&path,&st))
    ret = new MmdfMailbox(url_);
  if (MaildirMailbox::isMaildir(&path,&st))
    ret = new MaildirMailbox(url_);
  if (MHMailbox::isMH(&path,&st))
    ret = new MHMailbox(url_);

  buffer_free(&path);

  if (!ret && error) {
    buffer_add_str(error,_("unsupported mailbox format in '"),-1);
    buffer_add_str(error,url_->path,-1);
    buffer_add_ch(error,'\'');
  }

  return ret;
}
