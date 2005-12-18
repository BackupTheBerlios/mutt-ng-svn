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

PlainConnection::PlainConnection(url_t* url_) : Connection(url_) {
  buffer_init(&rbuf);
}

PlainConnection::~PlainConnection() {
  buffer_free(&rbuf);
}

int PlainConnection::doRead(buffer_t * buf, unsigned int len) {
  int read_len;

  buffer_grow(&rbuf,len);
  buffer_shrink(&rbuf,0);
  read_len = read(fd,rbuf.str,len);
  rbuf.str[len] = '\0';
  rbuf.len = len;

  switch (read_len) {
    case -1:
      is_connected = false;
      return -1;
    case  0:
      is_connected = false;
    default:
      buffer_shrink(buf,0);
      buffer_add_buffer(buf,&rbuf);
      break;
  }

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
