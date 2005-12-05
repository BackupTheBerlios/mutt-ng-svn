/** @ingroup libmuttng_message */
/**
 * @file libmuttng/message/body.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief MIME message body part
 */
#ifndef LIBMUTTNG_MESSGAGE_BODY_H
#define LIBMUTTNG_MESSGAGE_BODY_H

#include "libmuttng/libmuttng.h"

/** Single MIME body part */
class Body : public LibMuttng {
  public:
    Body (void);
    ~Body (void);
};

#endif /* !LIBMUTTNG_MESSGAGE_BODY_H */
