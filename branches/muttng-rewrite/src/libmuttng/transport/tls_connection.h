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
    TLSConnection(url_t* url_);
    ~TLSConnection();
    int doRead(buffer_t * buf, unsigned int len);
    int doWrite(buffer_t * buf);
    bool doOpen();
    bool doClose();
    /** @copydoc Connection::getVersion(). */
    static bool getVersion(buffer_t* dst);
    /** register gnutls specific stuff */
    static void reg();
  private:
    /** state of GNUTLS session. */
    gnutls_session state;
    /** credentials for managing certs. */
    gnutls_certificate_credentials xcred;
};

#endif
