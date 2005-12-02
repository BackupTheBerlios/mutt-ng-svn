/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/remote_mailbox.h
 * @brief Interface: Remote Mailbox base class
 */
#ifndef LIBMUTTNG_MAILBOX_REMOTE_MAILBOX__H
#define LIBMUTTNG_MAILBOX_REMOTE_MAILBOX__H

#include "mailbox.h"

#include "transport/connection.h"

/**
 * Mailbox with underlaying remote network connection.
 */
class RemoteMailbox : public Mailbox {
  public:
    RemoteMailbox (url_t* url_, Connection * c = NULL);
    ~RemoteMailbox ();
  protected:
    /** connection */
    Connection * conn;
    /** caching object */
    Cache* cache;
};

#endif
