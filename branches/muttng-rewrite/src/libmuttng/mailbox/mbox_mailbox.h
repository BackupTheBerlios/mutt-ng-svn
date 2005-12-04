/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/mbox_mailbox.h
 * @brief Interface: MBOX Mailbox class
 */
#ifndef LIBMUTTNG_MAILBOX_MBOX_MAILBOX_H
#define LIBMUTTNG_MAILBOX_MBOX_MAILBOX_H

#include "file_mailbox.h"

/**
 * MBOX Mailbox with an underlaying local storage.
 */
class MboxMailbox : public FileMailbox {
  public:
    MboxMailbox (url_t* url_);
    ~MboxMailbox ();

    bool isSeparator (void);

    void getSeparator (Message* msg);

    /**
     * Test whether local URL is a MBOX mailbox.
     * @return Yes/No.
     */
    static bool isMbox (url_t* url_);

}; 

#endif
