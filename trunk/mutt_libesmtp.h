#if !defined(LIBESMTP_H)
#define LIBESMTP_H

int
mutt_invoke_libesmtp (ADDRESS *from,	/* the sender */
		 ADDRESS *to, ADDRESS *cc, ADDRESS *bcc, /* recips */
		 const char *msg, /* file containing message */
		 int eightbit); /* message contains 8bit chars */

#endif /* !defined(LIBESMTP_H) */
