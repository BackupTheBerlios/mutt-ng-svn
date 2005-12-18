/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/mmdf_mailbox.h
 * @brief Interface: MMDF Mailbox class
 */
#ifndef LIBMUTTNG_MAILBOX_MMDF_MAILBOX_H
#define LIBMUTTNG_MAILBOX_MMDF_MAILBOX_H

#include "libmuttng/mailbox/file_mailbox.h"

/**
 * MMDF Mailbox with an underlaying local storage.
 */
class MmdfMailbox : public FileMailbox {
  public:
    /**
     * Create MMDF mailbox from URL.
     * @param url_ URL.
     */
    MmdfMailbox (url_t* url_);
    ~MmdfMailbox ();

    bool isSeparator (void);

    void getSeparator (Message* msg);

    /**
     * Test whether local URL is a MMDF mailbox.
     * @param url_ URL.
     * @param error Error buffer.
     * @return Yes/No.
     */
    static bool isMmdf (url_t* url_, buffer_t* error);

    unsigned long msgNew();
    unsigned long msgOld();
    unsigned long msgTotal();
    unsigned long msgFlagged();

}; 

#endif
