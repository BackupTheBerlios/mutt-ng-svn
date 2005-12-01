/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/pop3_mailbox.h
 * @brief Interface: POP3 Mailbox
 */
#ifndef LIBMUTTNG_MAILBOX_POP3_MAILBOX__H
#define LIBMUTTNG_MAILBOX_POP3_MAILBOX__H

#include "remote_mailbox.h"
#include "transport/connection.h"

/**
 * POP3 mailbox.
 */
class POP3Mailbox : public RemoteMailbox {
  public:
    POP3Mailbox (url_t* url_, Connection * c = NULL);
    ~POP3Mailbox ();

    const char* cacheKey (Message* msg);
    virtual Message* cacheLoadSingle (url_t* url, const char* key);
    virtual bool cacheDumpSingle (url_t* url, const char* key, Message* message);

    /**
     * Opens the POP3 mailbox.
     */
    virtual mailbox_query_status openMailbox();

    virtual bool checkEmpty();

    virtual bool checkACL(acl_bit_t bit);

    virtual mailbox_query_status closeMailbox();

    virtual mailbox_query_status syncMailbox();

    virtual mailbox_query_status checkMailbox();

    virtual mailbox_query_status commitMessage(Message *);

    virtual mailbox_query_status openNewMessage(Message *);

    virtual bool checkAccess();

    virtual mailbox_query_status fetchMessageHeaders(Message *, unsigned int);

    virtual mailbox_query_status fetchMessage(Message *, unsigned int);

};

#endif
