/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/pop3_mailbox.h
 * @brief Interface: POP3 Mailbox
 */
#ifndef LIBMUTTNG_MAILBOX_POP3_MAILBOX_H
#define LIBMUTTNG_MAILBOX_POP3_MAILBOX_H

#include "libmuttng/mailbox/remote_mailbox.h"
#include "libmuttng/transport/connection.h"

/**
 * POP3 mailbox.
 */
class POP3Mailbox : public RemoteMailbox {
  public:
    /**
     * Create POP3 mailbox from URL.
     * @param url_ URL.
     * @param c Connection.
     */
    POP3Mailbox (url_t* url_, Connection * c = NULL);
    ~POP3Mailbox ();

    /** register all POP3 specific config options */
    static void reg();

    /**
     * Opens the POP3 mailbox.
     */
    mailbox_query_status openMailbox();

    bool checkEmpty();

    bool checkACL(acl_bit_t bit);

    mailbox_query_status closeMailbox();

    mailbox_query_status syncMailbox();

    mailbox_query_status checkMailbox();

    mailbox_query_status commitMessage(Message * msg);

    mailbox_query_status openNewMessage(Message * msg);

    bool checkAccess();

    mailbox_query_status fetchMessageHeaders(Message * msg, unsigned int msgnum);

    mailbox_query_status fetchMessage(Message * msg, unsigned int msgnum);

    unsigned long msgNew();
    unsigned long msgOld();
    unsigned long msgTotal();
    unsigned long msgFlagged();

    /**
     * Compute key for caching a message.
     * This is connected to Cache::cacheGetKey.
     * @param msg Message to compute key for.
     * @param dst Destination storage for key.
     * @return true.
     */
    bool cacheGetKey (Message* msg, buffer_t* dst);
  private:
    unsigned long total;
};

#endif
