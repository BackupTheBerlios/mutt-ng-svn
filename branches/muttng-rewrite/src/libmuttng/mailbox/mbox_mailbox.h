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

    unsigned long msgNew();
    unsigned long msgOld();
    unsigned long msgTotal();
    unsigned long msgFlagged();

    /**
     * Test whether local URL is a MBOX mailbox.
     * @param path Path.
     * @param st Result of last stat(2) call.
     * @return Yes/No.
     */
    static bool isMbox (buffer_t* path, struct stat* st);

    /** register MBOX specific stuff */
    static void reg();

}; 

#endif
