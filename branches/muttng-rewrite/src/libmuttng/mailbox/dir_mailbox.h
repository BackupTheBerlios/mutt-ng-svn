/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/dir_mailbox.h
 * @brief Interface: Directory Mailbox base class
 *
 * This file is published under the GNU General Public License.
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
    /**
     * Create new directory-based mailbox from URL.
     * @param url_ url.
     */
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

    bool checkEmpty();

    bool checkACL(acl_bit_t bit);

    mailbox_query_status closeMailbox();

    mailbox_query_status syncMailbox();

    mailbox_query_status commitMessage(Message * msg);

    mailbox_query_status openNewMessage(Message * msg);

    bool checkAccess();

    mailbox_query_status fetchMessageHeaders(Message * msg, unsigned int msgnum);

    mailbox_query_status fetchMessage(Message * msg, unsigned int msgnum);

  protected:
    /** cache object */
    Cache* cache;
}; 

#endif
