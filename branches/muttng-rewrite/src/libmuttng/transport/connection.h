/** @ingroup libmuttng_util
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
#include "util/url.h"

#include "./../muttng_features.h"
#include "./../muttng_signal.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

/**
 * plain TCP connection.
 * Feel free to extend it to "plug-ins" so that SSL/TLS can be
 * transparently switched on.
 * pdmef: my choice is to have two flavors of read/write functions
 * whereby use is transparently hidden behind this interface and all is
 * done in this class to not have to deal with details anywhere outside
 * Connection class.
 */
class Connection {
  public:
    /**
     * Constructor for connection.
     * @param host Hostname.
     * @param port TCP destination port.
     * @param secure Whether to use a secure connection.
     */
    Connection(buffer_t * host = NULL, unsigned short port = 0,
               bool secure_ = false);
    ~Connection();

    /**
     * Start connection.
     * @return true if connection succeeded, false if connection failed.
     */
    bool connect();

    /**
     * Close connection.
     * @return true if close succeeded, false if connection failed.
     */
    bool disconnect();

    /**
     * Determines whether connection is secure or not.
     * @return Yes/No.
     */
    bool isSecure();

    /**
     * Set or unset whether secure connection is to be used.
     * @b Note: this works only while connection is closed/not opened yet.
     * @return Whether operation succeeded.
     */
    bool setSecure(bool secure_);

    /**
     * Reads a number of characters from the connection. If an error
     * occurs, the buffer is unaltered.
     * @param buf buffer into which the data should be read.
     * @param len number of characters to read.
     * @return number of characters read. -1 if an error occured.
     *         0 if the connection has been closed.
     */
    int doRead(buffer_t * buf, unsigned int len);

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
     * Reads a single character from the connection.
     * @return the character, or -1 if an error occurs.
     */
    int readChar();

    /**
     * Writes a buffer to the connection.
     * @return the number of characters written, or -1 if an
     *          error occurs.
     */
    int doWrite(buffer_t * buf);

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
    static Connection * fromURL(url_t * url_);

    /**
     * Signal emitted prior to opening the connection.
     * The values passed are:
     * -# hostname
     * -# port
     * -# whether connection will be secure
     */
    Signal3<buffer_t*,unsigned int,bool> sigPreconnect;

    /**
     * Signal emitted prior to accepting a certificate, if any.
     * This can be used by clients to decide whether to accept
     * or discard a certificate.
     * @todo pass certificate rather dummy int argument
     */
    Signal1<int> sigCheckCertificate;

  private:
    unsigned short tcp_port;
    buffer_t hostname;
    int fd; /* this will be pulled out when a pluggable transport
               mechanism system will be implemented */
    struct sockaddr_in sin;
    bool is_connected;
    bool secure;
};

#endif
/** @} */
