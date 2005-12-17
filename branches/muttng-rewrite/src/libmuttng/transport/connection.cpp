/** @ingroup libmuttng_transport */
/**
 * @file libmuttng/transport/connection.cpp
 * @author Andreas Krennmair <ak@synflood.at>
 * @brief Implementation: plain TCP connection
 * @todo implementation is completely missing.
 */
#include "libmuttng/libmuttng_features.h"

#include "connection.h"
#include "plain_connection.h"
#ifdef LIBMUTTNG_SSL_OPENSSL
#include "ssl_connection.h"
#endif
#ifdef LIBMUTTNG_SSL_GNUTLS
#include "tls_connection.h"
#endif

#include "core/mem.h"
#include "core/intl.h"
#include "core/str.h"

#include "util/url.h"

#include <stdio.h>
#include <errno.h>

#include <cstring>

#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <string.h>
#include <strings.h>

Connection::Connection(url_t* url_) : is_connected(false) {
  url = new url_t;
  url->username = str_dup(url_->username);
  url->password = str_dup(url_->password);
  url->host = str_dup(url_->host);
  url->port = url_->port;
  url->defport = url_->defport;
  url->secure = url_->secure;
  url->path = str_dup(url_->path);
  url->proto = url_->proto;
}

Connection::~Connection() {
  url_free(url);
  delete url;
}

bool Connection::socketConnect() {
  /*
   * TODO: we need to differentiate between different errors.
   * Shall we use exceptions to do that?
   */
  memset(&sin,0,sizeof(sin));

  if (!sigPreconnect.emit(url->host,url->port,url->secure))
    return false;

  struct hostent * hp = gethostbyname(url->host);

  if (NULL == hp) {
    printf("hp == NULL\n");
    return false;
  }

  sin.sin_family = AF_INET;

  bcopy(hp->h_addr, (char *) &sin.sin_addr, hp->h_length);
  sin.sin_port = htons(url->port);

  if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("socket failed: %s\n",strerror(errno));
    return false;
  }

  /*
   * just in case somebody asks, the "::" in this case is the scope
   * operator, and means "call the connect() function in the 'root' scope.",
   * i.e. the connect() function from the C library.
   */
  if(::connect(fd, (const struct sockaddr *) &sin, sizeof(sin)) < 0) {
    printf("connect failed: %s\n",strerror(errno));
    return false;
  }

  if (!doOpen()) {
    DEBUGPRINT(D_MOD,("doOpen() for %s:%d failed",url->host,url->port));
    return false;
  }

  is_connected = true;
  return true;
}

bool Connection::socketDisconnect() {
  if (shutdown(fd,SHUT_RDWR)<0) {
    return false;
  }
  if (close(fd)<0) {
    return false;
  }
  if (!doClose())
    return false;
  is_connected = false;
  return sigPostconnect.emit(url->host,url->port);
}

int Connection::readUntilSeparator(buffer_t * buf, char sep) {
  buffer_t rbuf;

  buffer_init(buf);
  buffer_init(&rbuf);

  do {
    buffer_shrink(&rbuf,0);
    int rc = doRead(&rbuf,sizeof(char));
    switch (rc) {
      case -1: 
        is_connected = false;
        return -1;
        break;
      case  0: 
        is_connected = false;
        return buf->len;
        break;
      default:
        buffer_add_ch(buf,rbuf.str[0]);
        break;
    }
  } while (sep != rbuf.str[0]);
  return buf->len;
}

int Connection::readLine(buffer_t * buf) {
  int rc = readUntilSeparator(buf,'\n');
  buffer_chomp(buf);
  DEBUGPRINT(D_SOCKET,(" %s:%d << '%s'",url->host,url->port,buf->str));
  return rc;
}

int Connection::writeLine(buffer_t * buf) {
  if (!buf) return 0;
  DEBUGPRINT(D_SOCKET,("%s:%d >> '%s'",url->host,url->port,buf->str));
  buffer_add_str(buf,"\r\n",2);
  return doWrite(buf);
}

bool Connection::canRead() {
  struct pollfd fds[] = { { fd, POLLIN, 0 } };
  if (poll(fds,1,0)>0) {
    if ((fds[0].revents & POLLIN)) {
      return true;
    }
  }
  return false;
}

bool Connection::isConnected() {
  return is_connected;
}

Connection * Connection::fromURL(url_t * url, buffer_t* error) {
  if (!url) {
    /* if not URL provided, we can't create a new connection. */
    buffer_init(error);
    buffer_add_str(error,_("failed to create connection: no URL given"),-1);
    return NULL;
  }
  if (!url->host || !url->port) {
    /* url with no host or port, can't create new connection either. */
    buffer_init(error);
    buffer_add_str(error,_("failed to create connection: no host and/or port given"),-1);
    return NULL;
  }

  Connection * conn = NULL;
#if defined(LIBMUTTNG_SSL_OPENSSL) || defined(LIBMUTTNG_SSL_GNUTLS)
  if (url->secure)
#ifdef LIBMUTTNG_SSL_OPENSSL
    conn = new SSLConnection(url);
#elif LIBMUTTNG_SSL_GNUTLS
    conn = new TLSConnection(url);
#else
    return NULL;
#endif
  else
#endif
    conn = new PlainConnection(url);

  return conn;
}

bool Connection::isSecure() {
  return url->secure;
}

url_t* Connection::getURL() {
  return url;
}

bool Connection::getSecureVersion(buffer_t* dst) {
#if defined(LIBMUTTNG_SSL_OPENSSL) || defined(LIBMUTTNG_SSL_GNUTLS)
#ifdef LIBMUTTNG_SSL_OPENSSL
  return SSLConnection::getVersion(dst);
#elif LIBMUTTNG_SSL_GNUTLS
  return TLSConnection::getVersion(dst);
#endif
#endif
  (void)dst;
  return false;
}
