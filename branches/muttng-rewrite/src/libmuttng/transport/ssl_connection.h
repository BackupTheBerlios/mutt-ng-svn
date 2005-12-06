/** @ingroup libmuttng_transport */
/**
 * @file libmuttng/transport/ssl_connection.h
 * @brief Interface: OpenSSL-based TCP connection
 */

#ifndef LIBMUTTNG_TRANSPORT_SSL_CONNECTION__H
#define LIBMUTTNG_TRANSPORT_SSL_CONNECTION__H

#include "connection.h"

/**
 * Abstract TCP connection.
 */
class SSLConnection : public Connection {
  public:
    SSLConnection(buffer_t * host = NULL, unsigned short port = 0,
               bool secure_ = false);
    ~SSLConnection();
    int doRead(buffer_t * buf, unsigned int len);
    int doWrite(buffer_t * buf);
    bool doOpen();
    bool doClose();
};

#endif
/** @} */
