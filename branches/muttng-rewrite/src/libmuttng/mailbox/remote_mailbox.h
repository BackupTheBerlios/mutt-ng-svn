/** @ingroup libmuttng_mailbox */
/**
 * @file remote_mailbox.h
 * @author Someone
 * @version $Id$
 * @brief Remote Mailbox interface.
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
