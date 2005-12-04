/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/remote_mailbox.h
 * @brief Interface: Remote Mailbox base class
 */
#ifndef LIBMUTTNG_MAILBOX_REMOTE_MAILBOX_H
#define LIBMUTTNG_MAILBOX_REMOTE_MAILBOX_H

#include "mailbox.h"

#include "transport/connection.h"

/**
 * Mailbox with underlaying remote network connection.
 */
class RemoteMailbox : public Mailbox {
  public:
    RemoteMailbox (url_t* url_, Connection * c = NULL);
    ~RemoteMailbox ();
    static Mailbox* fromURL (url_t* url_);
  protected:
    /** connection */
    Connection * conn;
    /** caching object */
    Cache* cache;
};

#endif
