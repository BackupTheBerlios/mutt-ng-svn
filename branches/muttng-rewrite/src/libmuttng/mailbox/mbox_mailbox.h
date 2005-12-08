/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/mbox_mailbox.h
 * @brief Interface: MBOX Mailbox class
 */
#ifndef LIBMUTTNG_MAILBOX_MBOX_MAILBOX_H
#define LIBMUTTNG_MAILBOX_MBOX_MAILBOX_H

#include "libmuttng/mailbox/file_mailbox.h"

/**
 * MBOX Mailbox with an underlaying local storage.
 */
class MboxMailbox : public FileMailbox {
  public:
    /**
     * Create MBOX mailbox from URL.
     * @param url_ URL.
     */
    MboxMailbox (url_t* url_);
    ~MboxMailbox ();

    bool isSeparator (void);

    void getSeparator (Message* msg);

    /**
     * Test whether local URL is a MBOX mailbox.
     * @param url_ URL.
     * @param error Error buffer.
     * @return Yes/No.
     */
    static bool isMbox (url_t* url_, buffer_t* error);

}; 

#endif
