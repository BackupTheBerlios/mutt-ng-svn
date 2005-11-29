/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/pop3_mailbox.h
 * @brief Interface: POP3 Mailbox
 */
#include "remote_mailbox.h"

/**
 * POP3 mailbox.
 */
class POP3Mailbox : public RemoteMailbox {
	public:
		POP3Mailbox ();
		~POP3Mailbox ();
		const char* key (Message* msg);

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

};
