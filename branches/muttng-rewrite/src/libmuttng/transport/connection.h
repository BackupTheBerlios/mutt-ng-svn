/**
 * @ingroup libmuttng_util
 * @addtogroup libmuttng_transport Connection
 * @{
 */
/**
 * @file libmuttng/transport/connection.h
 * @author Andreas Krennmair <ak@synflood.at>
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: plain TCP connection
 *
 * This file is published under the GNU General Public License.
 */

#ifndef LIBMUTTNG_TRANSPORT_CONNECTION__H
#define LIBMUTTNG_TRANSPORT_CONNECTION__H

#include "core/buffer.h"
#include "libmuttng/util/url.h"

#include "libmuttng/libmuttng.h"
#include "libmuttng/libmuttng_features.h"
#include "libmuttng/muttng_signal.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

typedef struct certinfo_t {
  char* name;
  char* contact;
  char* org;
  char* unit;
  char* location;
  char* state;
  char* country;
  certinfo_t() {
    name = NULL; contact = NULL;
    org = NULL; unit = NULL;
    location = NULL; state = NULL;
    country = NULL;
  }
} certinfo_t;

enum certcheck_t {
  CERT_REJECT = 0,
  CERT_SESSION,
  CERT_ALWAYS
};

/**
 * Abstract TCP connection.
 */
class Connection : public LibMuttng {
  public:
    /**
     * Constructor for connection.
     * @param url_ URL.
     */
    Connection(url_t* url_);
    virtual ~Connection();

    /**
     * Start connection.
     * @return true if connection succeeded, false if connection failed.
     * @test conn_tests::test_connectdisconnect().
     */
    bool socketConnect();

    /**
     * Close connection.
     * @return true if close succeeded, false if connection failed.
     * @test conn_tests::test_connectdisconnect().
     */
    bool socketDisconnect();

    /**
     * Determines whether connection is secure or not.
     * @return Yes/No.
     */
    bool isSecure();

    /**
     * Read bytes from connection.
     * @param buf Destination buffer.
     * @param len How many bytes to read.
     * @return
     *   - positive: number of bytes read
     *   - negative: error
     */
    virtual int doRead(buffer_t * buf, unsigned int len) = 0;

    /**
     * Write bytes to connection.
     * @param buf Source buffer.
     * @return
     *   - positive: number of bytes written
     *   - negative: error
     */
    virtual int doWrite(buffer_t * buf) = 0;

    /**
     * After opening the raw socket, perform this initialization.
     * @return Success.
     */
    virtual bool doOpen() = 0;

    /**
     * Before closing raw socket, perform this cleanup.
     * @return Success.
     */
    virtual bool doClose() = 0;

    /**
     * Reads characters from the connection until a separator is hit.
     * The separator is also added to the buffer.
     * @param buf buffer into which the data should be read.
     * @param sep the separator character.
     * @return number of characters read. -1 if an error occured.
     */
    int readUntilSeparator(buffer_t * buf, char sep);

    /**
     * Reads a complete line from the connection into a buffer. The end-of-line 
     * character(s) are stripped from the buffer.
     * @param buf buffer into which the data should be read.
     * @return number of characters read. -1 if an error occured.
     * @test conn_tests::test_readwrite().
     */
    int readLine(buffer_t * buf);

    /**
     * Writes buffer to connection. Also add \\r\\n line in any case.
     * @param buf Buffer with data to send.
     * @return number of characters written.
     * @test conn_tests::test_readwrite().
     */
    int writeLine(buffer_t* buf);

    /**
     * Determines whether there is data ready to be read.
     * @return true if there is data ready to be read, otherwise false.
     * @test conn_tests::test_canread().
     */
    bool canRead();

    /**
     * Determines whether this connection is currently connected.
     * @return true if connection is connected, otherwise false.
     */
    bool isConnected();

    /**
     * Creates a new connection based on the given URL.
     * @param url_ the URL object.
     * @param error Storage for URL parsing error.
     * @return new Connection, or NULL if an error occured.
     */
    static Connection * fromURL(url_t * url_, buffer_t* error);

    /**
     * Get version string of secure transport lib used.
     * @param dst Optional destination buffer.
     * @return Whether secure transport is supported.
     */
    static bool getSecureVersion(buffer_t* dst);

    /**
     * Retrieve URL for connection.
     * @return URL.
     */
    url_t* getURL();

    /**
     * Signal emitted prior to opening the connection.
     * The values passed are:
     * -# hostname
     * -# port
     * -# whether connection will be secure
     */
    Signal3<const char*,unsigned int,bool> sigPreconnect;

    /**
     * Signal emitted after closing the connection.
     * The values passed are:
     * -# hostname
     * -# port
     */
    Signal2<const char*,unsigned int> sigPostconnect;

    /** whether connection is authenticated and ready. */
    bool ready;

    /** register connection specific variables */
    static void reg();

    /** deregister connection specific items */
    static void dereg();

    /**
     * Signal emitted prior to accepting a certificate, if any.
     * This can be used by clients to decide whether to accept
     * or discard a certificate. The arguments are in order:
     * -# owner
     * -# issuer
     * -# fingerprint
     * -# valid since
     * -# valid until
     * -# what to do with certificate
     */
    static Signal6<certinfo_t*,certinfo_t*,const char*,const char*,const char*,certcheck_t*> sigCheckCertificate;

  protected:
    /** URL */
    url_t* url;
    /**
     * Socket file descriptor.
     * @bug this will be pulled out when a pluggable transport
     * mechanism system will be implemented
     */
    int fd; 
    /** whether connection is established */
    bool is_connected;

    /**
     * Free memory consumed by certinfo_t structure.
     * @param src Source.
     */
    void freeCertInfo(certinfo_t* src);

    /** Security Strength Factor for secure connections */
    int ssf;

  private:
    /** receive buffer */
    buffer_t rbuf;
    /** error message buffer */
    buffer_t errorMsg;
    /**
     * Fill error buffer with progress message. This is
     * "msg+host+..."
     * @param msg Message to which hostname is appended.
     * @return Length of string without "..." for shrinking buffer
     * and appending error message.
     */
    size_t makeMsg (const char* msg);
};

/**
 * @def WHERE
 * Neat trick to have storage+visibility of options: define storage location.
 */
#ifdef WHERE
#undef WHERE
#endif

/**
 * @def INITVAL(X).
 * Neat trick to have storage+visibility of options: set initial value.
 * @param X Inititial value.
 */
#ifdef INITVAL
#undef INITVAL
#endif

#ifdef CONNECTION_MAIN_CPP
#define WHERE
#define INITVAL(X)      =X
#else
#define WHERE extern
#define INITVAL(X)
#endif

/** storage for @ref option_ssl_client_cert */
WHERE char* SSLClientCert INITVAL(NULL);
/** storage for @ref option_ssl_certificate_file */
WHERE char* SSLCertFile INITVAL(NULL);
/** storage for @ref option_ssl_min_dh_prime_bits */
WHERE int SSLDHPrimeBits INITVAL(0);
/** storage for @ref option_ssl_ca_certificates_file */
WHERE char* SSLCaCertFile INITVAL(NULL);
/** storage for @ref option_ssl_use_tlsv1 */
WHERE bool UseTLS1 INITVAL(true);
/** storage for @ref option_ssl_use_sslv3 */
WHERE bool UseSSL3 INITVAL(true);

#endif
/** @} */
