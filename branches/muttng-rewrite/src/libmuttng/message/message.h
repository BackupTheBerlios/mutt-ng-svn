/** @ingroup libmuttng_message */
/**
 * @file libmuttng/message/message.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief MIME message
 *
 * This file is published under the GNU General Public License.
 */
#ifndef LIBMUTTNG_MESSAGE_MESSAGE_H
#define LIBMUTTNG_MESSAGE_MESSAGE_H

#include "libmuttng/libmuttng.h"

/** MIME message class */
class Message : public LibMuttng {
  public:
    Message (void);
    ~Message (void);
};

#endif /* !LIBMUTTNG_MESSAGE_MESSAGE_H */
