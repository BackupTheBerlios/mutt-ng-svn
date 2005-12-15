/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/nntp_mailbox.cpp
 * @brief Implementation: NNTP Mailbox
 */
#include <stdlib.h>

#include "core/str.h"

#include "nntp_mailbox.h"
#include "libmuttng/config/config_manager.h"

static char* DefaultUser = NULL;
static char* DefaultPassword = NULL;
static char* DefaultHost = NULL;

NNTPMailbox::NNTPMailbox (url_t* url_) : RemoteMailbox (url_) {
  this->haveCaching = 1;
  this->haveAuthentication = 1;
  this->haveEncryption = 1;
}

NNTPMailbox::~NNTPMailbox (void) {
}

void NNTPMailbox::reg() {
  char* p = getenv("NNTPSERVER");
  if (p)
    str_replace(&DefaultHost,p);
  /** @bug check /etc/nntpserver */

  ConfigManager::reg(new StringOption("nntp_user","",&DefaultUser));
  ConfigManager::reg(new StringOption("nntp_pass","",&DefaultPassword));
  ConfigManager::reg(new StringOption("nntp_host",NONULL(DefaultHost),&DefaultHost));
}
