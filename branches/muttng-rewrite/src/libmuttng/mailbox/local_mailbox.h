/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/local_mailbox.h
 * @brief Interface: Local Mailbox base class
 */
#ifndef LIBMUTTNG_MAILBOX_LOCAL_MAILBOX_H
#define LIBMUTTNG_MAILBOX_LOCAL_MAILBOX_H

#include "mailbox.h"

/**
 * Mailbox with an underlaying local storage.
 */
class LocalMailbox : public Mailbox {
  public:
    LocalMailbox (url_t* url_);
    ~LocalMailbox ();

    static Mailbox* fromURL (url_t* url_);
}; 

#endif
