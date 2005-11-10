/** @ingroup libmuttng_mailbox */
/**
 * @file local_mailbox.h
 * @author Someone
 * @version $Id$
 * @brief Local Mailbox interface.
 */
#include "mailbox.h"

/**
 * Mailbox with underlaying remote network connection.
 */
class LocalMailbox : public Mailbox {
  public:
    LocalMailbox ();
    ~LocalMailbox ();
};
