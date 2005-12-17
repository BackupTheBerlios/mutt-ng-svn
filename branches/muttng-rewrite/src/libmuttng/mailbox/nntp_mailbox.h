/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/nntp_mailbox.h
 * @brief Interface: NNTP Mailbox
 */
#ifndef LIBMUTTNG_MAILBOX_NNTP_MAILBOX_H
#define LIBMUTTNG_MAILBOX_NNTP_MAILBOX_H

#include "libmuttng/mailbox/remote_mailbox.h"
#include "libmuttng/transport/connection.h"

/**
 * NNTP mailbox.
 */
class NNTPMailbox : public RemoteMailbox {
  public:
    /**
     * Create NNTP mailbox from URL.
     * @param url_ URL.
     * @param c Connection.
     */
    NNTPMailbox (url_t* url_, Connection* c);
    ~NNTPMailbox ();

    /** register all NNTP specific config options */
    static void reg();

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

    /**
     * Compute key for caching a message.
     * This is connected to Cache::cacheGetKey.
     * @param msg Message to compute key for.
     * @param dst Destination storage for key.
     * @return true.
     */
    bool cacheGetKey (Message* msg, buffer_t* dst);

    /**
     * For a given URL, see if there's a usable one available already.
     * @param url_ URL.
     * @return Connection if found and @c NULL otherwise.
     */
    static Connection* findConnection(url_t* url_);

  private:
    /** whether server supports XPAT command */
    bool haveXPAT;
    /** whether server supports XGTITLE command */
    bool haveXGTITLE;
    /** whether server supports XOVER command */
    bool haveXOVER;
    /** whether server supports LISTGROUP command */
    bool haveLISTGROUP;
    /** whether server supports LIST NEWSGROUPS extension */
    bool haveLISTNEWSGROUPS;
    /** attempt to query for several extensions */
    bool getAllCapa();
    /**
     * Attempt to query for single extension.
     * @param capa Pointer to storage for capability.
     * @param cmd Which command to send.
     * @param cmdlen Length of command, ie strlen().
     * @return True if send/receive succeeded, false otherwise.
     */
    bool getSingleCapa(bool* capa, const char* cmd, size_t cmdlen);
    /** socket send buffer */
    buffer_t sbuf;
    /** socket receive buffer */
    buffer_t rbuf;
    /**
     * Attempt to authenticate if required.
     * @return Success.
     */
    bool auth();
    /**
     * Quit current connection.
     * @param state State to return.
     * @return State passed in.
     */
    mailbox_query_status quit(mailbox_query_status state);
    /**
     * Read reponse list and call function on it. If the callback
     * returns false, it won't be called any longer.
     * @param handler Optional callback.
     * @return
     *   - 0  (network) error
     *   - 1  ok, but no input received
     *   - >2 number of lines read+1
     */
    unsigned long readList(int (*handler)() = NULL);
};

#endif
