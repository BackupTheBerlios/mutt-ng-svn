/** @ingroup libmuttng_transport */
/**
 * @file libmuttng/transport/tls_connection.h
 * @author Copyright (C) 2001 Marco d'Itri <md@linux.it>
 * @author Copyright (C) 2001-2004 Andrew McDonald <andrew@mcdonald.org.uk>
 * @brief Interface: GNUTLS-based TCP connection
 */
#ifndef LIBMUTTNG_TRANSPORT_TLS_CONNECTION__H
#define LIBMUTTNG_TRANSPORT_TLS_CONNECTION__H

#include "connection.h"

#include <gnutls/openssl.h>

/**
 * TLS TCP connection.
 */
class TLSConnection : public Connection {
  public:
    /** @copydoc Connection::Connection(). */
    TLSConnection(buffer_t * host = NULL, unsigned short port = 0,
               bool secure_ = false);
    ~TLSConnection();
    int doRead(buffer_t * buf, unsigned int len);
    int doWrite(buffer_t * buf);
    bool doOpen();
    bool doClose();
  private:
    /**
     * Initialize GNUTLS.
     * @return Success.
     */
    bool init();
    /** whether we called init() already. */
    bool did_init;
    /** state of GNUTLS session. */
    gnutls_session state;
    /** credentials for managing certs. */
    gnutls_certificate_credentials xcred;
};

#endif
