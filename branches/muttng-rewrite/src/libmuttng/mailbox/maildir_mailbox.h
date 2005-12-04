/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/maildir_mailbox.h
 * @brief Interface: Maildir Mailbox base class
 */
#ifndef LIBMUTTNG_MAILBOX_MAILDIR_MAILBOX_H
#define LIBMUTTNG_MAILBOX_MAILDIR_MAILBOX_H

#include "dir_mailbox.h"

/**
 * Maildir Mailbox with an underlaying local storage.
 */
class MaildirMailbox : public DirMailbox {
  public:
    MaildirMailbox (url_t* url_);
    ~MaildirMailbox ();

    /**
      * Compute key for caching a message.
      * This is connected to Cache::cacheGetKey.
      * @param msg Message to compute key for.
      * @param dst Destination storage for key.
      * @return true.
      */
    bool cacheGetKey (Message* msg, buffer_t* dst);

    /**
     * Test whether local URL is a Maildir mailbox.
     * @return Yes/No.
     */
    static bool isMaildir (url_t* url_);
}; 

#endif
