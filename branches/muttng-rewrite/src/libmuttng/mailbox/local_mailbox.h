/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/local_mailbox.h
 * @brief Interface: Local Mailbox base class
 */
#include "mailbox.h"

/**
 * Mailbox with underlaying remote network connection.
 */
class LocalMailbox : public Mailbox {
  public:
    LocalMailbox ();
    ~LocalMailbox ();
    virtual const char* key (Message* msg) = 0;
};
