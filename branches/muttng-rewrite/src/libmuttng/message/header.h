/** @ingroup libmuttng_message */
/**
 * @file libmuttng/message/header.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief MIME message header
 */
#ifndef LIBMUTTNG_MESSGAGE_HEADER_H
#define LIBMUTTNG_MESSGAGE_HEADER_H

#include "../libmuttng.h"

/** Message header class */
class Header : public LibMuttng {
  public:
    Header (void);
    ~Header (void);
};

#endif /* !LIBMUTTNG_MESSGAGE_HEADER_H */
