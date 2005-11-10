/** @ingroup libmuttng_mailbox */
/**
 * @file pop3_mailbox.h
 * @author Someone
 * @version $Id$
 * @brief POP3 Mailbox interface.
 */
#include "remote_mailbox.h"

/**
 * POP3 mailbox.
 */
class POP3Mailbox : public RemoteMailbox {
  public:
    POP3Mailbox ();
    ~POP3Mailbox ();
};
