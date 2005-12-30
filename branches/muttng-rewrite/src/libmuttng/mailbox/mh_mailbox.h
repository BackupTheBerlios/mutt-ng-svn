/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/mh_mailbox.h
 * @brief Interface: MH Mailbox base class
 */
#ifndef LIBMUTTNG_MAILBOX_MH_MAILBOX_H
#define LIBMUTTNG_MAILBOX_MH_MAILBOX_H

#include "libmuttng/mailbox/dir_mailbox.h"

/**
 * MH Mailbox with an underlaying local storage.
 */
class MHMailbox : public DirMailbox {
  public:
    /**
     * Create MH mailbox from URL.
     * @param url_ URL.
     */
    MHMailbox (url_t* url_);
    ~MHMailbox ();

    /**
      * Compute key for caching a message.
      * This is connected to Cache::cacheGetKey.
      * @param msg Message to compute key for.
      * @param dst Destination storage for key.
      * @return true.
      */
    bool cacheGetKey (Message* msg, buffer_t* dst);

    /**
     * Test whether local URL is a MH mailbox.
     * @param path Path.
     * @param st Result of last stat(2) call.
     * @return Yes/No.
     */
    static bool isMH (buffer_t* path, struct stat* st);

    mailbox_query_status openMailbox();

    mailbox_query_status checkMailbox();

    unsigned long msgNew();
    unsigned long msgOld();
    unsigned long msgTotal();
    unsigned long msgFlagged();

    /** register MH specific stuff */
    static void reg();

}; 

#endif
