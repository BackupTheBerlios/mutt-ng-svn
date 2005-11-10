/** @ingroup libmuttng_mailbox */
/**
 * @file imap_mailbox.h
 * @author Someone
 * @version $Id$
 * @brief IMAP Mailbox interface.
 */
#include "remote_mailbox.h"

/**
 * IMAP mailbox.
 */
class ImapMailbox : public RemoteMailbox {
  public:
    ImapMailbox ();
    ~ImapMailbox ();
};
