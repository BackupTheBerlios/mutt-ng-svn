/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/imap_mailbox.h
 * @brief Interface: IMAP Mailbox
 */
#include "remote_mailbox.h"

/**
 * IMAP mailbox.
 */
class ImapMailbox : public RemoteMailbox {
  public:
    ImapMailbox (url_t* url_);
    ~ImapMailbox ();
    const char* cacheKey (Message* msg);
};
