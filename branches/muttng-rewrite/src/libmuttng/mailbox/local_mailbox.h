/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/local_mailbox.h
 * @brief Interface: Local Mailbox base class
 */
#ifndef LIBMUTTNG_MAILBOX_LOCAL_MAILBOX_H
#define LIBMUTTNG_MAILBOX_LOCAL_MAILBOX_H

#include "libmuttng/mailbox/mailbox.h"

/**
 * Mailbox with an underlaying local storage.
 */
class LocalMailbox : public Mailbox {
  public:
    /**
     * Create new local mailbox from URL.
     * @param url_ url.
     */
    LocalMailbox (url_t* url_);
    ~LocalMailbox ();

    /**
     * Create local mailbox from URL.
     * @param url_ URL.
     * @param error Error buffer.
     * @return Mailbox or @c NULL in case of error.
     */
    static Mailbox* fromURL (url_t* url_,buffer_t* error);
}; 

#endif
