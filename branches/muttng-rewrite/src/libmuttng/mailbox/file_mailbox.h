/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/file_mailbox.h
 * @brief Interface: Single file Mailbox base class
 */
#ifndef LIBMUTTNG_MAILBOX_FILE_MAILBOX_H
#define LIBMUTTNG_MAILBOX_FILE_MAILBOX_H

#include "local_mailbox.h"

#include "message/message.h"

/**
 * Single file-based Mailbox with an underlaying local storage.
 */
class FileMailbox : public LocalMailbox {
  public:
    FileMailbox (url_t* url_);
    ~FileMailbox ();

    /**
     * See if line passed is a separator line.
     * @return Yes/No.
     */
    virtual bool isSeparator (void) = 0;

    /**
     * For a given message, construct separator line.
     */
    virtual void getSeparator (Message* msg) = 0;

    mailbox_query_status openMailbox();

    bool checkEmpty();

    bool checkACL(acl_bit_t bit);

    mailbox_query_status closeMailbox();

    mailbox_query_status syncMailbox();

    mailbox_query_status checkMailbox();

    mailbox_query_status commitMessage(Message *);

    mailbox_query_status openNewMessage(Message *);

    bool checkAccess();

    mailbox_query_status fetchMessageHeaders(Message *, unsigned int);

    mailbox_query_status fetchMessage(Message *, unsigned int);

}; 

#endif