/** @ingroup libmuttng_message */
/**
 * @file libmuttng/message/simple_header.h
 * @brief Generic MIME message header
 *
 * This file is published under the GNU General Public License.
 */
#ifndef LIBMUTTNG_MESSAGE_SIMPLE_HEADER_H
#define LIBMUTTNG_MESSAGE_SIMPLE_HEADER_H

#include "libmuttng/message/header.h"

/**
 * Generic or simple header without much specialization.
 */
class SimpleHeader : public Header {
  public:
    SimpleHeader(char * name = NULL, char * body = NULL);
    SimpleHeader(buffer_t * name, buffer_t * body);
    bool decode();
    bool encode();
};

#endif /* !LIBMUTTNG_MESSAGE_HEADER_H */
