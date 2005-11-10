/** @ingroup libmuttng_mailbox */
/**
 * @file nntp_mailbox.h
 * @author Someone
 * @version $Id$
 * @brief NNTP Mailbox interface.
 */
#include "remote_mailbox.h"

/**
 * NNTP mailbox.
 */
class NNTPMailbox : public RemoteMailbox {
  public:
    NNTPMailbox ();
    ~NNTPMailbox ();
};
