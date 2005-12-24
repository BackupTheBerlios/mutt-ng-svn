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

#include "libmuttng/config/config_manager.h"

#include "local_mailbox.h"

#include "mbox_mailbox.h"
#include "mmdf_mailbox.h"
#include "maildir_mailbox.h"
#include "mh_mailbox.h"

/** storage for @ref option_local_mail_check */
static int Timeout = 0;
/** storage for @ref option_mbox_type */
static char* DefaultType = NULL;

LocalMailbox::LocalMailbox (url_t* url_) : Mailbox (url_) {
  this->haveFilters = 1;
}

LocalMailbox::~LocalMailbox (void) {
}

void LocalMailbox::reg() {
  Option* opt = ConfigManager::reg(new IntOption("local_mail_check","30",&Timeout,false));
  ConfigManager::reg(new SynOption("mail_check",opt));
  ConfigManager::reg(new StringOption("mbox_type","maildir",&DefaultType,"^(mbox|mmdf|maildir|mh)$"));
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
