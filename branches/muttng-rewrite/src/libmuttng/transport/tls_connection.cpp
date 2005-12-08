/** @ingroup libmuttng_transport */
/*
 * @file libmuttng/transport/tls_connection.cpp
 * @author Copyright (C) 2001 Marco d'Itri <md@linux.it>
 * @author Copyright (C) 2001-2004 Andrew McDonald <andrew@mcdonald.org.uk>
 * @brief Implementation: GNUTLS-based TCP connection
 */
#include "tls_connection.h"

#include <gnutls/openssl.h>

#include "core/mem.h"

#include <unistd.h>

/** array of protocols priorities for negotiation */
static int priority[] = { GNUTLS_TLS1, GNUTLS_SSL3, 0 };

TLSConnection::TLSConnection(buffer_t * host, unsigned short port,
                                 bool secure_) : Connection(host,port,secure_),
                                                 did_init(false) {}
TLSConnection::~TLSConnection() {}

bool TLSConnection::init() {
  if (did_init)
    return true;
  int err = gnutls_global_init();
  if (err < 0) {
    DEBUGPRINT(D_MOD,("gnutls_global_init(): %d, %s",err,gnutls_strerror(err)));
    return false;
  }
  did_init = true;
  return true;
}

int TLSConnection::doRead(buffer_t * buf, unsigned int len) {
  if (!is_connected)
    return -1;

  char* rbuf = (char*) mem_malloc(len+1);
  int ret = gnutls_record_recv(state,rbuf,len);

  if (gnutls_error_is_fatal(ret) == 1) {
    DEBUGPRINT(D_MOD,("gnutls_record_recv(): %d, %s",ret,gnutls_strerror(ret)));
    mem_free(&rbuf);
    return -1;
  }
  buffer_add_str(buf,rbuf,ret);
  DEBUGPRINT(D_SOCKET,("%s:%d << '%s'",hostname.str,tcp_port,rbuf));
  mem_free(&rbuf);
  return ret;
}

int TLSConnection::doWrite(buffer_t * buf) {
  if (!buf) return -1;

  int rc = write(fd,buf->str,buf->len);

  DEBUGPRINT(D_SOCKET,("%s:%d >> '%s'",hostname.str,tcp_port,buf->str));

  if (rc<0) {
    is_connected = false;
  }

  return rc;
}

bool TLSConnection::doOpen() {
  int err = gnutls_certificate_allocate_credentials (&xcred);
  if (err < 0) {
    DEBUGPRINT(D_MOD,("gnutls_certificate_allocate_credentials: %s",
                      gnutls_strerror(err)));
    return false;
  }

  gnutls_certificate_set_x509_trust_file (xcred, /* XXX SslCertFile */ NULL,
                                          GNUTLS_X509_FMT_PEM);
  /* ignore errors, maybe file doesn't exist yet */

  if (/* XXX SslCACertFile */ NULL) {
    gnutls_certificate_set_x509_trust_file (xcred, /* XXX SslCACertFile */ NULL,
                                            GNUTLS_X509_FMT_PEM);
  }

  gnutls_init (&state, GNUTLS_CLIENT);

  /* set socket */
  gnutls_transport_set_ptr (state, (gnutls_transport_ptr) fd);

  /* XXX disable TLS/SSL protocols as needed
  if (!option (OPTTLSV1) && !option (OPTSSLV3)) {
    mutt_error (_("All available protocols for TLS/SSL connection disabled"));
    goto fail;
  }
  else if (!option (OPTTLSV1)) {
    priority[0] = GNUTLS_SSL3;
    priority[1] = 0;
  }
  else if (!option (OPTSSLV3)) {
    priority[0] = GNUTLS_TLS1;
    priority[1] = 0;
  }
  */

  /* We use default priorities (see gnutls documentation),
     except for protocol version */
  gnutls_set_default_priority (state);
  gnutls_protocol_set_priority (state, priority);

  if (/* XXX SslDHPrimeBits */ 0 > 0) {
    gnutls_dh_set_prime_bits (state, /* XXX SslDHPrimeBits */ 0);
  }

  gnutls_credentials_set (state, GNUTLS_CRD_CERTIFICATE, xcred);

  err = gnutls_handshake (state);

  while (err == GNUTLS_E_AGAIN || err == GNUTLS_E_INTERRUPTED) {
    err = gnutls_handshake (state);
  }
  if (err < 0) {
    if (err == GNUTLS_E_FATAL_ALERT_RECEIVED) {
      DEBUGPRINT(D_MOD,("gnutls_handshake: %s(%s)", gnutls_strerror (err),
                 gnutls_alert_get_name (gnutls_alert_get (state))));
    }
    else {
      DEBUGPRINT(D_MOD,("gnutls_handshake: %s", gnutls_strerror (err)));
    }
    goto fail;
  }

  if (!sigCheckCertificate.emit(/* XXX */ 0))
    goto fail;

  /* set Security Strength Factor (SSF) for SASL */
  /* NB: gnutls_cipher_get_key_size() returns key length in bytes */
  ssf = gnutls_cipher_get_key_size (gnutls_cipher_get (state)) * 8;

  DEBUGPRINT(D_MOD,("SSL/TLS connection using %s (%s/%s/%s)",
                    gnutls_protocol_get_name (gnutls_protocol_get_version
                                              (state)),
                    gnutls_kx_get_name (gnutls_kx_get (state)),
                    gnutls_cipher_get_name (gnutls_cipher_get (state)),
                    gnutls_mac_get_name (gnutls_mac_get (state))));
  return /* XXX */ false;

fail:
  gnutls_certificate_free_credentials (xcred);
  gnutls_deinit (state);
  return false;
}

bool TLSConnection::doClose() {
  if (is_connected) {
    gnutls_bye (state, GNUTLS_SHUT_RDWR);
    gnutls_certificate_free_credentials (xcred);
    gnutls_deinit (state);
    return true;
  }
  return false;
}
