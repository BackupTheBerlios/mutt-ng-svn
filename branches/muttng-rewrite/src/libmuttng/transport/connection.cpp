/** @ingroup libmuttng_transport */
/**
 * @file libmuttng/transport/connection.cpp
 * @author Andreas Krennmair <ak@synflood.at>
 * @brief Implementation: plain TCP connection
 * @todo implementation is completely missing.
 */

#include "connection.h"

#include "core/mem.h"
#include "core/intl.h"

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

Connection::Connection(buffer_t * host, unsigned short port, bool secure_) :
  tcp_port(port),  is_connected(false), secure(secure_) {

  buffer_init(&hostname);
  buffer_add_buffer(&hostname,host);
}

Connection::~Connection() {
  buffer_free(&hostname);
}

bool Connection::connect() {
  /*
   * TODO: we need to differentiate between different errors.
   * Shall we use exceptions to do that?
   */
  memset(&sin,0,sizeof(sin));

  sigPreconnect.emit (&hostname, tcp_port, secure);

  struct hostent * hp = gethostbyname(hostname.str);

  if (NULL == hp) {
    printf("hp == NULL\n");
    return false;
  }

  sin.sin_family = AF_INET;

  bcopy(hp->h_addr, (char *) &sin.sin_addr, hp->h_length);
  sin.sin_port = htons(tcp_port);

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

  /* XXX do SSL/TLS right init/connect() right here */
#if 0
  if sslsetup
    ...
  else
    return false;
#endif

  /* XXX don't pass fd here but some representation of certificate */
  if (secure && !sigCheckCertificate.emit (fd)) {
    disconnect();
    return false;
  }

  is_connected = true;
  return true;
}

bool Connection::disconnect() {
  if (shutdown(fd,SHUT_RDWR)<0) {
    return false;
  }
  if (close(fd)<0) {
    return false;
  }
  is_connected = false;
  sigPostconnect.emit (&hostname, tcp_port);
  return true;
}

int Connection::doRead(buffer_t * buf, unsigned int len) {
  char* cbuf;
  int read_len;

  cbuf = (char*) mem_malloc (len+1);
  read_len = read(fd,cbuf,len);

  DEBUGPRINT(D_SOCKET,("%s:%d << '%s'",hostname.str,tcp_port,cbuf));

  switch (read_len) {
    case -1:
      is_connected = false;
      mem_free (&cbuf);
      return -1;
    case  0:
      is_connected = false;
    default:
      buffer_shrink(buf,0);
      buffer_add_str(buf,cbuf,read_len);
      break;
  }

  mem_free (&cbuf);
  return read_len;
}

int Connection::readUntilSeparator(buffer_t * buf, char sep) {
  buffer_init(buf);
  char c;

  do {
    int rc = read(fd,&c,sizeof(c));
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
        buffer_add_ch(buf,c);
        break;
    }
  } while (sep != c);
  DEBUGPRINT(D_SOCKET,("%s:%d << '%s'",hostname.str,tcp_port,buf->str));
  return buf->len;
}

int Connection::readLine(buffer_t * buf) {
  return readUntilSeparator(buf,'\n');
}

int Connection::readChar() {
  char c;
  if (read(fd,&c,sizeof(c))<=0) {
    is_connected = false;
    return -1;
  }
  return c;
}

int Connection::doWrite(buffer_t * buf) {
  if (!buf) return -1;

  int rc = write(fd,buf->str,buf->len);

  DEBUGPRINT(D_SOCKET,("%s:%d >> '%s'",hostname.str,tcp_port,buf->str));

  if (rc<0) {
    is_connected = false;
  }

  return rc;
}

unsigned short Connection::port() {
  return tcp_port;
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

  buffer_t hostbuf;
  buffer_init(&hostbuf);
  buffer_add_str(&hostbuf,url->host,-1);

  Connection * conn = new Connection(&hostbuf,url->port,url->secure);

  buffer_free(&hostbuf);

  return conn;
}

bool Connection::isSecure() {
  return secure;
}

bool Connection::setSecure (bool secure_) {
  if (is_connected)
    return false;
  secure = secure_;
  return true;
}
