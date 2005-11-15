/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/remote_mailbox.h
 * @brief Interface: Remote Mailbox base class
 */
#include "mailbox.h"

/**
 * Mailbox with underlaying remote network connection.
 */
class RemoteMailbox : public Mailbox {
  public:
    RemoteMailbox ();
    ~RemoteMailbox ();
};
