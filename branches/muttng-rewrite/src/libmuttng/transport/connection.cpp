/** @ingroup libmuttng_transport */
/**
 * @file libmuttng/transport/connection.cpp
 * @author Andreas Krennmair <ak@synflood.at>
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: plain TCP connection
 * @todo implementation is completely missing.
 */
#include "libmuttng/libmuttng_features.h"

#define CONNECTION_MAIN_CPP
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
#include "core/sigs.h"

#include "libmuttng/util/url.h"
#include "libmuttng/config/config_manager.h"

#include <stdio.h>
#include <errno.h>

#include <cstring>

#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <time.h>
#include <string.h>
#include <strings.h>

#include <iostream>
#include <vector>

/** storage for @ref option_connect_timeout */
static int ConnectTimeout = 0;
/** Connections */
static std::vector<Connection*> Connections;

Connection::Connection(url_t* url_) : ready(false), is_connected(false) {
  url = new url_t;
  url->username = str_dup(url_->username);
  url->password = str_dup(url_->password);
  url->host = str_dup(url_->host);
  url->idn_host = str_dup(url_->idn_host);
  url->port = url_->port;
  url->defport = url_->defport;
  url->secure = url_->secure;
  url->path = str_dup(url_->path);
  url->proto = url_->proto;
  buffer_init(&errorMsg);
  buffer_init(&rbuf);
}

Connection::~Connection() {
  url_free(url);
  delete url;
  buffer_free(&rbuf);
  buffer_free(&errorMsg);
}

void Connection::reg() {
  ConfigManager::reg(new IntOption("connect_timeout","30",&ConnectTimeout,false));
  ConfigManager::reg(new IntOption("ssl_min_dh_prime_bits","0",&SSLDHPrimeBits,false));
  ConfigManager::reg(new StringOption("ssl_client_cert","",&SSLClientCert));
  /* XXX SYN */
  Option* opt = ConfigManager::reg(new StringOption("ssl_certificate_file","$HOME/.mutt_certificates",&SSLCertFile));
  ConfigManager::reg(new SynOption("certificate_file",opt));
  ConfigManager::reg(new StringOption("ssl_ca_certificates_file","",&SSLCaCertFile));
  ConfigManager::reg(new BoolOption("ssl_use_sslv3","true",&UseSSL3));
  ConfigManager::reg(new BoolOption("ssl_use_tlsv1","true",&UseTLS1));
}

void Connection::dereg() {
  while(Connections.size()!=0) {
    Connection* conn = Connections.back();
    Connections.pop_back();
    delete conn;
  }
}

size_t Connection::makeMsg (const char* msg) {
  buffer_shrink(&errorMsg,0);
  buffer_add_str(&errorMsg,msg,-1);
  buffer_add_str(&errorMsg," '",2);
  buffer_add_str(&errorMsg,url->host,-1);
  buffer_add_ch(&errorMsg,'\'');
  size_t ret = errorMsg.len;
  buffer_add_str(&errorMsg,"...",3);
  return ret;
}

bool Connection::socketConnect() {
  /*
   * TODO: we need to differentiate between different errors.
   * Shall we use exceptions to do that?
   */
  buffer_shrink(&errorMsg,0);

  if (!sigPreconnect.emit(url->host,url->port,url->secure))
    return false;

  size_t msglen = makeMsg(_("Looking up host"));
  displayProgress->emit(&errorMsg);

  buffer_t port;
  buffer_init(&port);
  buffer_grow(&port,5);
  buffer_add_snum(&port,url->port,-1);

  struct addrinfo hints, *res, *r;
  int error;

  memset(&hints,0,sizeof(hints));
  hints.ai_family = PF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if ((error = getaddrinfo(url->idn_host,port.str,&hints,&res))<0) {
    buffer_shrink(&errorMsg,msglen);
    buffer_add_str(&errorMsg,_(": "),-1);
    buffer_add_str(&errorMsg,gai_strerror(error),-1);
    displayError->emit(&errorMsg);
    buffer_free(&port);
    return false;
  }

  buffer_free(&port);

  if (NULL==res) {
    buffer_shrink(&errorMsg,msglen);
    buffer_add_str(&errorMsg,_(": "),-1);
    buffer_add_str(&errorMsg,_("no addresses."),-1);
    displayError->emit(&errorMsg);
    return false;
  }

  msglen = makeMsg(_("Connecting to"));
  displayProgress->emit(&errorMsg);

  for (r = res; r; r = r->ai_next) {
    if ((fd = socket(r->ai_family,r->ai_socktype,0))<0)
      continue;
    if (ConnectTimeout > 0)
      alarm(ConnectTimeout);
    if (::connect(fd,r->ai_addr,r->ai_addrlen)<0) {
      alarm(0);
      close(fd);
      continue;
    }
    alarm(0);
    is_connected = true;
    freeaddrinfo(res);
    break;
  }

  if (!is_connected) {
    freeaddrinfo(res);
    buffer_shrink(&errorMsg,msglen);
    buffer_add_str(&errorMsg,_(": "),-1);
    buffer_add_str(&errorMsg,strerror(errno),-1);
    displayError->emit(&errorMsg);
    return false;
  }

  if (!doOpen()) {
    DEBUGPRINT(D_MOD,("doOpen() for %s:%d failed",url->host,url->port));
    is_connected = false;
    return false;
  }

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
  buffer_shrink(buf,0);
  int rc = readUntilSeparator(buf,'\n');
  buffer_chomp(buf);
  DEBUGPRINT(D_SOCKET,(" %s:%d:%ld < '%s'",url->host,url->port,time(NULL),buf->str));
  return rc;
}

int Connection::writeLine(buffer_t * buf) {
  if (!buf) return 0;
  DEBUGPRINT(D_SOCKET,("%s:%d:%ld > '%s'",url->host,url->port,time(NULL),buf->str));
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

  size_t i;
  /* try to find "cached" connection */
  for (i=0; i<Connections.size(); i++) {
    if (url_eq(Connections[i]->getURL(),url,false))
      return Connections[i];
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

  if (conn)
    Connections.push_back(conn);

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
