/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/mh_mailbox.h
 * @brief Interface: MH Mailbox base class
 */
#ifndef LIBMUTTNG_MAILBOX_MH_MAILBOX_H
#define LIBMUTTNG_MAILBOX_MH_MAILBOX_H

#include "dir_mailbox.h"

/**
 * MH Mailbox with an underlaying local storage.
 */
class MHMailbox : public DirMailbox {
  public:
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
     * @return Yes/No.
     */
    static bool isMH (url_t* url_);
}; 

#endif
