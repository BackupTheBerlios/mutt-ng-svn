/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/imap_mailbox.h
 * @brief Interface: IMAP Mailbox
 */
#ifndef LIBMUTTNG_MAILBOX_IMAP_MAILBOX_H
#define LIBMUTTNG_MAILBOX_IMAP_MAILBOX_H

#include "remote_mailbox.h"

/**
 * IMAP mailbox.
 */
class ImapMailbox : public RemoteMailbox {
  public:
    ImapMailbox (url_t* url_);
    ~ImapMailbox ();
}; 

#endif
