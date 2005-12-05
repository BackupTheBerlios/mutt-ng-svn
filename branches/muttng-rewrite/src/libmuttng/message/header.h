/** @ingroup libmuttng_message */
/**
 * @file libmuttng/message/header.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @author Andreas Krennmair <ak@synflood.at>
 * @brief MIME message header
 */
#ifndef LIBMUTTNG_MESSGAGE_HEADER_H
#define LIBMUTTNG_MESSGAGE_HEADER_H

#include "../libmuttng.h"
#include "core/buffer.h"

/**
 * Message header class. Implements an internet message header
 * according to RFC 2822.
 */
class Header : public LibMuttng {
  public:
    /**
    * Constructor.
    * @param name header name as C string.
    * @param body header body as C string.
    */
    Header(char * name = "", char * body = "");

    /**
    * Constructor.
    * @param name header name as @c buffer_t pointer.
    * @param body header body as @c buffer_t pointer.
    */
    Header(buffer_t * name, buffer_t * body);

    /**
     * Destructor.
     */
    ~Header (void);

    /**
     * Compares the header name with a string in a case-insensitive way.
     * @param name header name to compare with.
     * @return true if name equals the header name, otherwise false.
     */
    bool equalsName(char * name);

    /**
     * Compares the header name with a buffer_t in a case-insensitive way.
     * @param name header name to compare with.
     * @return true if name equals the header name, otherwise false.
     */
    bool equalsName(buffer_t * name);

    /**
     * Getter function for header name.
     * @return header name as @c buffer_t pointer.
     */
    buffer_t * getName();

    /**
     * Getter function for header body.
     * @return header body as @c buffer_t pointer.
     */
    buffer_t * getBody();

    /**
     * Setter function for header name.
     * @param name new header name.
     */
    void setName(buffer_t * name);

    /**
     * Setter function for header body.
     * @param body new header body.
     */
    void setBody(buffer_t * body);
  private:
    buffer_t name;
    buffer_t body;
};

#endif /* !LIBMUTTNG_MESSGAGE_HEADER_H */
