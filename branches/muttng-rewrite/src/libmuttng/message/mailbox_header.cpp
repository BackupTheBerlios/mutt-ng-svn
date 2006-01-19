/** @ingroup libmuttng_message */
/**
 * @file libmuttng/message/mailbox_header.cpp
 * @brief Generic MIME message header
 *
 * This file is published under the GNU General Public License.
 */
#include "mailbox_header.h"

MailboxHeader::MailboxHeader(char* name, char* body) : Header(name,body) {}

MailboxHeader::MailboxHeader(buffer_t* name, buffer_t* body) : Header(name,body) {}

bool MailboxHeader::encode() { return true; }
bool MailboxHeader::decode() { return true; }
