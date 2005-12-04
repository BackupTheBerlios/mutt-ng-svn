/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/nntp_mailbox.h
 * @brief Interface: NNTP Mailbox
 */
#ifndef LIBMUTTNG_MAILBOX_NNTP_MAILBOX_H
#define LIBMUTTNG_MAILBOX_NNTP_MAILBOX_H

#include "remote_mailbox.h"

/**
 * NNTP mailbox.
 */
class NNTPMailbox : public RemoteMailbox {
  public:
    NNTPMailbox (url_t* url_);
    ~NNTPMailbox ();
};

#endif
