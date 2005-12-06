/** @ingroup libmuttng_transport */
/**
 * @file libmuttng/transport/ssl_connection.cpp
 * @brief Implementation: OpenSSL-based TCP connection
 */
#include "ssl_connection.h"

#include "core/mem.h"

#include <unistd.h>

SSLConnection::SSLConnection(buffer_t * host, unsigned short port,
                                 bool secure_) : Connection(host,port,secure_) {}
SSLConnection::~SSLConnection() {}

int SSLConnection::doRead(buffer_t * buf, unsigned int len) {
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

int SSLConnection::doWrite(buffer_t * buf) {
  if (!buf) return -1;

  int rc = write(fd,buf->str,buf->len);

  DEBUGPRINT(D_SOCKET,("%s:%d >> '%s'",hostname.str,tcp_port,buf->str));

  if (rc<0) {
    is_connected = false;
  }

  return rc;
}

bool SSLConnection::doOpen() { return false; }
bool SSLConnection::doClose() { return false; }
