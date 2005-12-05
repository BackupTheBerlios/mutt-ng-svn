/**
 * @ingroup libmuttng
 * @addtogroup libmuttng_mailbox Mailbox
 * @{
 */
/**
 * @file libmuttng/mailbox/mailbox.h
 * @brief Interface: Mailbox base class
 */
#ifndef LIBMUTTNG_MAILBOX_MAILBOX_H
#define LIBMUTTNG_MAILBOX_MAILBOX_H

#include "libmuttng/libmuttng.h"
#include "libmuttng/message/message.h"
#include "libmuttng/util/url.h"
#include "libmuttng/cache/cache.h"

/**
 * ACLs for mailbox access.
 * For IMAP, a list is obtained from the server. For other protocols, these may
 * be hard-coded.
 */
enum acl_bit_t {
  /** lookup subscribed folders/new messages */
  ACL_LOOKUP = 0,
  /** read/search messages */
  ACL_READ,
  /** mark as read */
  ACL_SEEN,
  /** change any flags except "deleted" and "read" */
  ACL_WRITE,
  /** editing/appending messages */
  ACL_INSERT,
  /** save message */
  ACL_POST,
  /** create/rename/remove folders */
  ACL_CREATE,
  /** mark as (un)deleted and commit deletion */
  ACL_DELETE,
  /** change ACLs */
  ACL_ADMIN,
  /** for static array sizes */
  RIGHTSMAX
};

/**
 * State to be returned by operations on mailboxes.
 */
enum mailbox_query_status {
  /** error */
  MQ_ERR = -3,
  /** not authenticated */
  MQ_AUTH = -2,
  /** problem with connection */
  MQ_NOT_CONNECTED = -1,
  /** means that operation went OK */
  MQ_OK = 0,
  /** means that the Mailbox::checkNewMail() operation
   * detected new mail in the mailbox. */
  MQ_NEW_MAIL = 1
};

/**
 * Base class for mailbox abstraction.
 */
class Mailbox : public LibMuttng {
  public:
    /** construct mailbox form URL already */
    Mailbox (url_t* url_);
    /** cleanup */
    virtual ~Mailbox ();
    /**
     * Get URL for mailbox.
     * @param dst Destination buffer.
     */
    void getUrl (buffer_t* dst);

    /**
     * Opens the mailbox.
     * @return
     * - mailbox_query_status::MQ_OK if open was successful, 
     * - mailbox_query_status::MQ_NOT_CONNECTED if the connection failed or the mailbox was otherwise unavailable, and
     * - mailbox_query_status::MQ_ERR if login failed.
     */
    virtual mailbox_query_status openMailbox() = 0;

    /**
     * Checks if mailbox is empty.
     * @return true if mailbox is empty, otherwise false.
     */
    virtual bool checkEmpty() = 0;

    /**
     * Checks ACL bit.
     * @param bit Bit to check.
     * @return true if operation is granted through ACL, otherwise false.
     */
    virtual bool checkACL(acl_bit_t bit) = 0;

    /**
     * Closes the mailbox.
     * @return
     * - mailbox_query_status::MQ_OK if close was successful,
     * - mailbox_query_status::MQ_NOT_CONNECTED if the mailbox was disconnected before logging out, and
     * - mailbox_query_status::MQ_ERR if the logout was not successful.
     */
    virtual mailbox_query_status closeMailbox() = 0;

    /**
     * Synchronizes the mailbox.
     * @return
     * - mailbox_query_status::MQ_OK if sync was successful,
     * - mailbox_query_status::MQ_NOT_CONNECTED if the connection was lost during sync, and
     * - mailbox_query_status::MQ_ERR if sync was not successful due to some other error.
     */
    virtual mailbox_query_status syncMailbox() = 0;

    /**
     * Check mailbox for new mail.
     * @return
     * - mailbox_query_status::MQ_OK if no new mails are in the mailbox,
     * - mailbox_query_status::MQ_NEW_MAIL if new mail arrived in the mailbox,
     * - mailbox_query_status::MQ_NOT_CONNECTED if the connection was lost during check, and
     * - mailbox_query_status::MQ_ERR if some other error happened during check.
     */
    virtual mailbox_query_status checkMailbox() = 0;

    /**
     * Commit message to mailbox.
     * @param msg Message to commit.
     * @return
     * - mailbox_query_status::MQ_OK if commit was successful,
     * - mailbox_query_status::MQ_NOT_CONNECTED if the connection was lost during check, and
     * - mailbox_query_status::MQ_ERR if some other error happened during check.
     */
    virtual mailbox_query_status commitMessage(Message * msg) = 0;

    /**
     * Open a new (local temporary, as far as I understand XXX) message.
     * @param msg Message.
     * @return
     * - mailbox_query_status::MQ_OK if open was successful,
     * - mailbox_query_status::MQ_ERR otherwise.
     */
    virtual mailbox_query_status openNewMessage(Message * msg) = 0;

    /**
     * Check whether access to mailbox is possible.
     * @return true if access is possible, otherwise false.
     */
    virtual bool checkAccess() = 0;

    /**
     * Fetch message headers of message into Message object.
     * @param msg Message object into which the message headers shall be fetched.
     * @param msgnum message number.
     * @return
     * - mailbox_query_status::MQ_OK if fetch was successful,
     * - mailbox_query_status::MQ_NOT_CONNECTED if mailbox was not connected,
     * - mailbox_query_status::MQ_ERR otherwise.
     */
    virtual mailbox_query_status fetchMessageHeaders(Message * msg, unsigned int msgnum) = 0;

    /**
     * Fetch complete message into Message object.
     * @param msg Message object into which the message headers shall be fetched.
     * @param msgnum message number.
     * @return
     * - mailbox_query_status::MQ_OK if fetch was successful,
     * - mailbox_query_status::MQ_NOT_CONNECTED if mailbox was not connected,
     * - mailbox_query_status::MQ_ERR otherwise.
     */
    virtual mailbox_query_status fetchMessage(Message * msg, unsigned int msgnum) = 0;

    /**
     * Abstract interface to different modules: create Mailbox
     * object depending on URL.
     * @param url URL string.
     * @param error Error buffer where error message will be put (if any).
     * @return Mailbox object or @c NULL in case of error.
     */
    static Mailbox* fromURL (const char* url, buffer_t* error);

    /**
     * Retrieve error message for state.
     * @param State state.
     * @return Error message.
     */
    static const char* strerror (mailbox_query_status state);

    /**
     * Signal emitted when username for connection is required.
     * Parameters in order are:
     * -# url
     */
    Signal1<url_t*> sigGetUsername;

    /**
     * Signal emitted when password for connection is required.
     * Parameters in order are:
     * -# url
     */
    Signal1<url_t*> sigGetPassword;

  protected:
    /** whether mailbox supports caching */
    unsigned int haveCaching:1;
    /** whether mailbox supports authencation */
    unsigned int haveAuthentication:1;
    /** whether mailbox supports encryption */
    unsigned int haveEncryption:1;
    /** whether mailbox supports filters */
    unsigned int haveFilters:1;
    /** URL if any. */
    url_t* url;
};

#endif /* !LIBMUTTNG_MAILBOX_MAILBOX_H */
/** @} */
