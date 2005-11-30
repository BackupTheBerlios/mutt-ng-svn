/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/local_mailbox.h
 * @brief Interface: Local Mailbox base class
 */
#include "mailbox.h"

/**
 * Mailbox with an underlaying local storage.
 */
class LocalMailbox : public Mailbox {
  public:
    LocalMailbox (url_t* url_);
    ~LocalMailbox ();
    virtual const char* cacheKey (Message* msg) = 0;
};
