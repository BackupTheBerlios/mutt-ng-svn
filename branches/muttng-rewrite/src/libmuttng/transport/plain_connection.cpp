/** @ingroup libmuttng_transport */
/**
 * @file libmuttng/transport/plain_connection.cpp
 * @author Andreas Krennmair <ak@synflood.at>
 * @brief Implementation: plain TCP connection
 * @todo implementation is completely missing.
 */
#include "plain_connection.h"

#include "core/mem.h"

#include <unistd.h>

PlainConnection::PlainConnection(buffer_t * host, unsigned short port,
                                 bool secure_) : Connection(host,port,secure_) {}
PlainConnection::~PlainConnection() {}

int PlainConnection::doRead(buffer_t * buf, unsigned int len) {
  char* cbuf;
  int read_len;

  cbuf = (char*) mem_malloc (len+1);
  read_len = read(fd,cbuf,len);
  cbuf[len] = '\0';

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

int PlainConnection::doWrite(buffer_t * buf) {
  if (!buf) return -1;

  int rc = write(fd,buf->str,buf->len);

  if (rc<0) {
    is_connected = false;
  }

  return rc;
}

bool PlainConnection::doOpen() { return true; }
bool PlainConnection::doClose() { return true; }
