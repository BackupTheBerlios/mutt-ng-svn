/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/mh_mailbox.cpp
 * @brief Implementation: MH Mailbox base class
 */
#include "mh_mailbox.h"

#include <unistd.h>

MHMailbox::MHMailbox(url_t* url_) : DirMailbox(url_) {}

MHMailbox::~MHMailbox() {}

bool MHMailbox::cacheGetKey (Message* msg, buffer_t* dst) {
  (void)msg;
  (void)dst;
  return false;
}

bool MHMailbox::isMH (buffer_t* path, struct stat* st) {
  if (!S_ISDIR(st->st_mode)) return false;
  size_t l = path->len;
  static const char* files[] = { "/.mh_sequences", "/.xmhcache", "/.new_cache",
                                 "/.new-cache", "./sylpheed_cache", "/.overview", NULL };
  unsigned short i = 0;
  while (files[i]) {
    buffer_shrink(path,l);
    buffer_add_str(path,files[i++],-1);
    if (access(path->str,F_OK)==0) {
      buffer_shrink(path,l);
      return true;
    }
  }
  return false;
}

mailbox_query_status MHMailbox::openMailbox() { return MQ_ERR; }

mailbox_query_status MHMailbox::checkMailbox() { return MQ_ERR; }

unsigned long MHMailbox::msgNew() { return 0; }
unsigned long MHMailbox::msgOld() { return 0; }
unsigned long MHMailbox::msgTotal() { return 0; }
unsigned long MHMailbox::msgFlagged() { return 0; }
