/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/nntp_mailbox.h
 * @brief Interface: NNTP Mailbox
 */
#include "remote_mailbox.h"

/**
 * NNTP mailbox.
 */
class NNTPMailbox : public RemoteMailbox {
  public:
    NNTPMailbox ();
    ~NNTPMailbox ();
    const char* key (Message* msg);
};
