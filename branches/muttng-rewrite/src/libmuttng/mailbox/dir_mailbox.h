/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/dir_mailbox.h
 * @brief Interface: Directory Mailbox base class
 */
#ifndef LIBMUTTNG_MAILBOX_DIR_MAILBOX_H
#define LIBMUTTNG_MAILBOX_DIR_MAILBOX_H

#include "libmuttng/mailbox/local_mailbox.h"
#include "libmuttng/cache/cache.h"
#include "libmuttng/message/message.h"

/**
 * Directory--based Mailbox with an underlaying local storage.
 */
class DirMailbox : public LocalMailbox {
  public:
    DirMailbox (url_t* url_);
    ~DirMailbox ();

    /**
      * Compute key for caching a message.
      * This is connected to Cache::cacheGetKey.
      * @param msg Message to compute key for.
      * @param dst Destination storage for key.
      * @return true.
      */
    virtual bool cacheGetKey (Message* msg, buffer_t* dst) = 0;

    mailbox_query_status openMailbox();

    bool checkEmpty();

    bool checkACL(acl_bit_t bit);

    mailbox_query_status closeMailbox();

    mailbox_query_status syncMailbox();

    mailbox_query_status checkMailbox();

    mailbox_query_status commitMessage(Message *);

    mailbox_query_status openNewMessage(Message *);

    bool checkAccess();

    mailbox_query_status fetchMessageHeaders(Message *, unsigned int);

    mailbox_query_status fetchMessage(Message *, unsigned int);

  protected:
    Cache* cache;
}; 

#endif
