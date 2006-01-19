/** @ingroup libmuttng_message */
/**
 * @file libmuttng/message/body.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief MIME message body part
 *
 * This file is published under the GNU General Public License.
 */
#ifndef LIBMUTTNG_MESSAGE_BODY_H
#define LIBMUTTNG_MESSAGE_BODY_H

#include "libmuttng/libmuttng.h"

/** Single MIME body part */
class Body : public LibMuttng {
  public:
    Body (void);
    ~Body (void);
};

#endif /* !LIBMUTTNG_MESSAGE_BODY_H */
