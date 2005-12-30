/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/imap_mailbox.h
 * @brief Interface: IMAP Mailbox
 */
#ifndef LIBMUTTNG_MAILBOX_IMAP_MAILBOX_H
#define LIBMUTTNG_MAILBOX_IMAP_MAILBOX_H

#include "libmuttng/mailbox/remote_mailbox.h"

/**
 * IMAP mailbox.
 */
class ImapMailbox : public RemoteMailbox {
  public:
    /**
     * Create new IMAP mailbox from URL.
     * @param url_ url.
     */
    ImapMailbox (url_t* url_);
    ~ImapMailbox ();
    /** register IMAP specific stuff */
    static void reg();
}; 

#endif
