/**
 * @ingroup libmuttng_util
 * @addtogroup libmuttng_transport Connection
 * @{
 */
/**
 * @file libmuttng/transport/connection.h
 * @author Andreas Krennmair <ak@synflood.at>
 * @brief Interface: plain TCP connection
 * @todo implementation is completely missing.
 */

#ifndef LIBMUTTNG_TRANSPORT_CONNECTION__H
#define LIBMUTTNG_TRANSPORT_CONNECTION__H

#include "core/buffer.h"
#include "libmuttng/util/url.h"

#include "libmuttng/libmuttng.h"
#include "libmuttng/muttng_features.h"
#include "libmuttng/muttng_signal.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

/**
 * Abstract TCP connection.
 */
class Connection : public LibMuttng {
  public:
    /**
     * Constructor for connection.
     * @param host Hostname.
     * @param port TCP destination port.
     * @param secure Whether to use a secure connection.
     */
    Connection(buffer_t * host = NULL, unsigned short port = 0,
               bool secure_ = false);
    virtual ~Connection();

    /**
     * Start connection.
     * @return true if connection succeeded, false if connection failed.
     */
    bool socketConnect();

    /**
     * Close connection.
     * @return true if close succeeded, false if connection failed.
     */
    bool socketDisconnect();

    /**
     * Determines whether connection is secure or not.
     * @return Yes/No.
     */
    bool isSecure();

    virtual int doRead(buffer_t * buf, unsigned int len) = 0;
    virtual int doWrite(buffer_t * buf) = 0;
    virtual bool doOpen() = 0;
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
     * character(s) are also appended to the buffer.
     * @param buf buffer into which the data should be read.
     * @return number of characters read. -1 if an error occured.
     */
    int readLine(buffer_t * buf);

    /**
     * Read accessor for tcp_port.
     * @return the currently set TCP port.
     */
    unsigned short port();

    /**
     * Determines whether there is data ready to be read.
     * @return true if there is data ready to be read, otherwise false.
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
     * @return new Connection, or NULL if an error occured.
     */
    static Connection * fromURL(url_t * url_, buffer_t* error);

    /**
     * Signal emitted prior to opening the connection.
     * The values passed are:
     * -# hostname
     * -# port
     * -# whether connection will be secure
     */
    Signal3<buffer_t*,unsigned int,bool> sigPreconnect;

    /**
     * Signal emitted after closing the connection.
     * The values passed are:
     * -# hostname
     * -# port
     */
    Signal2<buffer_t*,unsigned int> sigPostconnect;

  protected:
    unsigned short tcp_port;
    buffer_t hostname;
    int fd; /* this will be pulled out when a pluggable transport
               mechanism system will be implemented */
    struct sockaddr_in sin;
    bool is_connected;
    bool secure;
    /**
     * Signal emitted prior to accepting a certificate, if any.
     * This can be used by clients to decide whether to accept
     * or discard a certificate.
     * @todo pass certificate rather dummy int argument
     */
    Signal1<int> sigCheckCertificate;
    int ssf;
};

#endif
/** @} */
