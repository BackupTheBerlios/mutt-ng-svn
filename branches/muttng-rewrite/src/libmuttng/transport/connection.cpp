#include "connection.h"

#include <stdio.h>
#include <errno.h>

#include <cstring>

#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/poll.h>

Connection::Connection(buffer_t * host, unsigned short port) : tcp_port(port) {
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

  return true;
}

bool Connection::disconnect() {
  if (shutdown(fd,SHUT_RDWR)<0) {
    return false;
  }
  if (close(fd)<0) {
    return false;
  }
  return true;
}

int Connection::doRead(buffer_t * buf, unsigned int len) {
  char cbuf[len+1];
  int read_len;

  read_len = read(fd,cbuf,len);

  if (read_len==-1) {
    return -1;
  }

  buffer_shrink(buf,0);
  buffer_add_str(buf,cbuf,read_len);

  return read_len;
}

int Connection::readUntilSeparator(buffer_t * buf, char sep) {
  buffer_init(buf);
  char c;

  do {
    int rc = read(fd,&c,sizeof(c));
    switch (rc) {
      case -1: 
        return -1;
        break;
      case  0: 
        return buf->len;
        break;
      default:
        buffer_add_ch(buf,c);
        break;
    }
  } while (sep != c);
  return buf->len;
}

int Connection::readLine(buffer_t * buf) {
  return readUntilSeparator(buf,'\n');
}

int Connection::readChar() {
  char c;
  if (read(fd,&c,sizeof(c))<0) {
    return -1;
  }
  return c;
}

int Connection::doWrite(buffer_t * buf) {
  if (!buf) return -1;

  return write(fd,buf->str,buf->len);
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
