/**
 * @ingroup libmuttng
 * @addtogroup libmuttng_mailbox Mailbox
 * @{
 */
/**
 * @file libmuttng/mailbox/mailbox.h
 * @brief Interface: Mailbox base class
 */
#include "../cache/cache.h"

enum acl_bit_t {
	ACL_LOOKUP = 0,
	ACL_READ,
	ACL_SEEN,
	ACL_WRITE,
	ACL_INSERT,
	ACL_POST,
	ACL_CREATE,
	ACL_DELETE,
	ACL_ADMIN,

	RIGHTSMAX
};

enum mailbox_query_status {
	MQ_ERR = -2,		/** error */
	MQ_NOT_CONNECTED = -1,	/** problem with connection */
	MQ_OK = 0,		/** means that operation went OK */
	MQ_NEW_MAIL = 1		/** means that the checkNewMail() operation detected new mail in the mailbox. */
};

/**
 * Base class for mailbox abstraction.
 */
class Mailbox : public Cache {
  public:
    /** construct mailbox form URL already */
    Mailbox (const char* url = NULL);
    /** cleanup */
    ~Mailbox ();
    /**
     * Get URL for mailbox.
     * @return URL or @c NULL if none set (yet.)
     */
    const char* getUrl ();
    /**
     * Implementation of Cache::key().
     */
    virtual const char* key (Message* msg) = 0;

    /**
     * Opens the mailbox.
     * @return MQ_OK if open was successful, 
     * 		MQ_NOT_CONNECTED if the connection failed or the
     * 		mailbox was otherwise unavailable, and
     * 		MQ_ERR if login failed.
     */
    virtual mailbox_query_status openMailbox() = 0;

    /**
     * Checks if mailbox is empty.
     * @return true if mailbox is empty, otherwise false.
     */
    virtual bool checkEmpty() = 0;

    /**
     * Checks ACL bit.
     * @return true if operation is granted through ACL, otherwise false.
     */
    virtual bool checkACL(acl_bit_t bit) = 0;

    /**
     * Closes the mailbox.
     * @return MQ_OK if close was successful, MQ_NOT_CONNECTED if the
     * 		mailbox was disconnected before logging out, and
     * 		MQ_ERR if the logout was not successful.
     */
    virtual mailbox_query_status closeMailbox() = 0;

    /**
     * Synchronizes the mailbox.
     * @return MQ_OK if sync was successful, MQ_NOT_CONNECTED if the
     * 		connection was lost during sync, and
     * 		MQ_ERR if sync was not successful due to some other
     * 		error.
     */
    virtual mailbox_query_status syncMailbox() = 0;

    /**
     * Check mailbox for new mail.
     * @return MQ_OK if no new mails are in the mailbox, MQ_NEW_MAIL if
     *		new mail arrived in the mailbox, MQ_NOT_CONNECTED if
     *		the connection was lost during check, and MQ_ERR
     *		if some other error happened during check.
     */
    virtual mailbox_query_status checkMailbox() = 0;

    /**
     * Commit message to mailbox.
     * @return MQ_OK if commit was successful, MQ_NOT_CONNECTED if
     * 		the connection was lost during check, and MQ_ERR
     * 		if some other error happened during check.
     */
    virtual mailbox_query_status commitMessage(Message *) = 0;

    /**
     * Open a new (local temporary, as far as I understand XXX) message.
     * @return MQ_OK if open was successful, MQ_ERR otherwise.
     */
    virtual mailbox_query_status openNewMessage(Message *) = 0;

    /**
     * Check whether access to mailbox is possible.
     * @return true if access is possible, otherwise false.
     */
    virtual bool checkAccess() = 0;

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
    const char* url;
};

/** @} */
