/** @ingroup libmuttng_message */
/**
 * @file libmuttng/message/mailbox_header.h
 * @brief Mailbox MIME message header
 */
#ifndef LIBMUTTNG_MESSAGE_MAILBOX_HEADER_H
#define LIBMUTTNG_MESSAGE_MAILBOX_HEADER_H

#include "libmuttng/message/header.h"

/**
 * Header transparently handling address lists as content.
 * This is split from a Generic header to hide all the
 * RfC2822 encoding/decoding from the Header class.
 */
class MailboxHeader : public Header {
  public:
    MailboxHeader(char * name = NULL, char * body = NULL);
    MailboxHeader(buffer_t * name, buffer_t * body);
    bool decode();
    bool encode();
};

#endif /* !LIBMUTTNG_MESSAGE_HEADER_H */
