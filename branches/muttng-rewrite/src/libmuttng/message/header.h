/** @ingroup libmuttng_message */
/**
 * @file libmuttng/message/header.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @author Andreas Krennmair <ak@synflood.at>
 * @brief MIME message header
 *
 * This file is published under the GNU General Public License.
 */
#ifndef LIBMUTTNG_MESSAGE_HEADER_H
#define LIBMUTTNG_MESSAGE_HEADER_H

#include "libmuttng/libmuttng.h"
#include "libmuttng/util/rfc2047.h"
#include "core/buffer.h"

/**
 * Message header class.
 */
class Header : protected RfC2047 {
  public:
    /**
    * Constructor. If one of the parameters is NULL, then the according member is initialized with an empty string.
    * @param name header name as C string.
    * @param body header body as C string.
    * @test header_tests::test_constructors()
    */
    Header(char * name = NULL, char * body = NULL);

    /**
    * Constructor. If one of the parameters is NULL, then the according members is initialized with an empty string.
    * @param name header name as @c buffer_t pointer.
    * @param body header body as @c buffer_t pointer.
    * @test header_tests::test_constructors()
    */
    Header(buffer_t * name, buffer_t * body);

    /**
     * Destructor.
     */
    virtual ~Header (void);

    /**
     * Decode value from whatever implementing class wants.
     * @return Success.
     */
    virtual bool decode() = 0;

    /**
     * Encode value to whatever implementing class wants.
     * @return Success.
     */
    virtual bool encode() = 0;

    /**
     * Compares the header name with a string in a case-insensitive way.
     * @param name header name to compare with.
     * @return true if name equals the header name, otherwise false. name == NULL always returns false.
     * @test header_tests::test_equalsname()
     */
    bool equalsName(char * name);

    /**
     * Compares the header name with a buffer_t in a case-insensitive way.
     * @param name header name to compare with.
     * @return true if name equals the header name, otherwise false. name == NULL always returns false.
     * @test header_tests::test_equalsname()
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
     * @param name new header name. If it is NULL, the header name is not modified.
     */
    void setName(buffer_t * name);

    /**
     * Setter function for header body.
     * @param body new header body. If it is NULL, the header name is not modified.
     */
    void setBody(buffer_t * body);

    /**
     * Serialization function.
     * @param buf buffer_t into which serialization shall happen.
     * @test header_tests::test_serialization()
     */
    void serialize(buffer_t * buf);

    /**
     * Parsing function.
     * @param buf buffer_t from which the header will be parsed.
     * @test header_tests::test_parse()
     */
    void parseLine(buffer_t * buf);

  private:
    /** header's field name */
    buffer_t name;
  protected:
    /** header's actual content */
    buffer_t body;
};


#endif /* !LIBMUTTNG_MESSAGE_HEADER_H */
