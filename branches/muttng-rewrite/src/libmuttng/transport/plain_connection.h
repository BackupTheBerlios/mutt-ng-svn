/** @ingroup libmuttng_transport */
/**
 * @file libmuttng/transport/plain_connection.h
 * @author Andreas Krennmair <ak@synflood.at>
 * @brief Interface: plain TCP connection
 *
 * This file is published under the GNU General Public License.
 */

#ifndef LIBMUTTNG_TRANSPORT_PLAIN_CONNECTION__H
#define LIBMUTTNG_TRANSPORT_PLAIN_CONNECTION__H

#include "connection.h"

/**
 * Abstract TCP connection.
 */
class PlainConnection : public Connection {
  public:
    /** @copydoc Connection::Connection(). */
    PlainConnection(url_t* url_);
    ~PlainConnection();
    int doRead(buffer_t * buf, unsigned int len);
    int doWrite(buffer_t * buf);
    bool doOpen();
    bool doClose();
};

#endif
