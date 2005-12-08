/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/remote_mailbox.h
 * @brief Interface: Remote Mailbox base class
 */
#ifndef LIBMUTTNG_MAILBOX_REMOTE_MAILBOX_H
#define LIBMUTTNG_MAILBOX_REMOTE_MAILBOX_H

#include "core/buffer.h"

#include "libmuttng/mailbox/mailbox.h"
#include "libmuttng/transport/connection.h"

/**
 * Mailbox with underlaying remote network connection.
 */
class RemoteMailbox : public Mailbox {
  public:
    /**
     * Create Maildir mailbox from URL.
     * @param url_ URL.
     * @param c Connection.
     */
    RemoteMailbox (url_t* url_, Connection * c = NULL);
    ~RemoteMailbox ();
    /**
     * Create remote mailbox from URL.
     * @param url_ URL.
     * @param error Error buffer.
     * @return Mailbox or @c NULL in case of error.
     */
    static Mailbox* fromURL (url_t* url_,buffer_t* error);
  protected:
    /** connection */
    Connection * conn;
    /** caching object */
    Cache* cache;
};

#endif
