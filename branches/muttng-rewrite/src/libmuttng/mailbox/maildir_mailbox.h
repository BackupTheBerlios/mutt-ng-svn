/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/maildir_mailbox.h
 * @brief Interface: Maildir Mailbox base class
 */
#ifndef LIBMUTTNG_MAILBOX_MAILDIR_MAILBOX_H
#define LIBMUTTNG_MAILBOX_MAILDIR_MAILBOX_H

#include "libmuttng/mailbox/dir_mailbox.h"

/**
 * Maildir Mailbox with an underlaying local storage.
 */
class MaildirMailbox : public DirMailbox {
  public:
    /**
     * Create Maildir mailbox from URL.
     * @param url_ URL.
     */
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
     * @param url_ URL.
     * @param error Error buffer.
     * @return Yes/No.
     */
    static bool isMaildir (buffer_t* path, struct stat* st);

    mailbox_query_status openMailbox();

    mailbox_query_status checkMailbox();

    unsigned long msgNew();
    unsigned long msgOld();
    unsigned long msgTotal();
    unsigned long msgFlagged();

  private:
    void parseFlags (const char* path);
    unsigned long mNew;
    unsigned long mTotal;
    unsigned long mFlagged;
};

#endif
