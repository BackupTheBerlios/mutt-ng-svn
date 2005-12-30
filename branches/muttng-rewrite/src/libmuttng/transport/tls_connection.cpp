/** @ingroup libmuttng_transport */
/**
 * @file libmuttng/transport/tls_connection.cpp
 * @author Copyright (C) 2001 Marco d'Itri <md@linux.it>
 * @author Copyright (C) 2001-2004 Andrew McDonald <andrew@mcdonald.org.uk>
 * @brief Implementation: GNUTLS-based TCP connection
 */
#include "tls_connection.h"

#include "libmuttng/config/config_manager.h"

#include <gnutls/openssl.h>

#include "core/mem.h"
#include "core/intl.h"

#include <unistd.h>

/** array of protocols priorities for negotiation */
static int priority[] = { GNUTLS_TLS1, GNUTLS_SSL3, 0 };
/** whether GNUTLS was initialized globally */
static bool did_init = false;

/** initialize gnutls library */
static void init () {
  if (did_init) return;
  did_init = gnutls_global_init()>=0;
  /** @bug handle errors */
}

TLSConnection::TLSConnection(url_t* url_) : Connection(url_) {
  if (!did_init) init();
}

TLSConnection::~TLSConnection() {}

void TLSConnection::reg() {
  ConfigManager::regFeature("ssl");
  ConfigManager::regFeature("gnutls");
}

int TLSConnection::doRead(buffer_t * buf, unsigned int len) {
  if (!is_connected)
    return -1;

  buffer_grow(buf,len+1);
  buffer_shrink(buf,0);
  int ret = gnutls_record_recv(state,buf->str,len);

  if (gnutls_error_is_fatal(ret) == 1) {
    is_connected = false;
    DEBUGPRINT(D_MOD,("gnutls_record_recv(): %d, %s",ret,gnutls_strerror(ret)));
    return -1;
  }
  return ret;
}

int TLSConnection::doWrite(buffer_t * buf) {
  if (!buf) return -1;
  int ret = gnutls_record_send(state,buf->str,buf->len);
  if (gnutls_error_is_fatal(ret) == 1) {
    is_connected = false;
    DEBUGPRINT(D_MOD,("gnutls_record_send(): %d, %s",ret,gnutls_strerror(ret)));
    return -1;
  }
  return ret;
}

bool TLSConnection::doOpen() {
  int err = gnutls_certificate_allocate_credentials (&xcred);
  if (err < 0) {
    DEBUGPRINT(D_MOD,("gnutls_certificate_allocate_credentials: %s",
                      gnutls_strerror(err)));
    return false;
  }

  gnutls_certificate_set_x509_trust_file (xcred, SSLCertFile,
                                          GNUTLS_X509_FMT_PEM);
  /* ignore errors, maybe file doesn't exist yet */

  if (SSLCaCertFile) {
    gnutls_certificate_set_x509_trust_file (xcred, SSLCaCertFile,
                                            GNUTLS_X509_FMT_PEM);
  }

  gnutls_init (&state, GNUTLS_CLIENT);

  /* set socket */
  gnutls_transport_set_ptr (state, (gnutls_transport_ptr) fd);

  /* disable TLS/SSL protocols as needed */
  if (!UseTLS1 && !UseSSL3) {
    buffer_t tmp;
    buffer_init(&tmp);
    buffer_add_str(&tmp,_("All available protocols for TLS/SSL connection disabled"),-1);
    displayError.emit(&tmp);
    buffer_free(&tmp);
    return doClose();
  }
  else if (!UseTLS1) {
    priority[0] = GNUTLS_SSL3;
    priority[1] = 0;
  }
  else if (!UseSSL3) {
    priority[0] = GNUTLS_TLS1;
    priority[1] = 0;
  }

  /* We use default priorities (see gnutls documentation),
     except for protocol version */
  gnutls_set_default_priority (state);
  gnutls_protocol_set_priority (state, priority);

  if (SSLDHPrimeBits > 0) {
    gnutls_dh_set_prime_bits (state, SSLDHPrimeBits);
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
    return doClose();
  }

  /* XXX 
  if (!sigCheckCertificate.emit(0))
    return doClose(); */

  /* set Security Strength Factor (SSF) for SASL */
  /* NB: gnutls_cipher_get_key_size() returns key length in bytes */
  ssf = gnutls_cipher_get_key_size (gnutls_cipher_get (state)) * 8;

  buffer_t msg;
  buffer_init(&msg);
  buffer_add_str(&msg,_("SSL/TLS connection using "),-1);
  buffer_add_str(&msg,gnutls_protocol_get_name(gnutls_protocol_get_version(state)),-1);
  buffer_add_str(&msg," (",2);
  buffer_add_str(&msg,gnutls_kx_get_name(gnutls_kx_get(state)),-1);buffer_add_ch(&msg,'/');
  buffer_add_str(&msg,gnutls_cipher_get_name(gnutls_cipher_get(state)),-1);buffer_add_ch(&msg,'/');
  buffer_add_str(&msg,gnutls_mac_get_name(gnutls_mac_get(state)),-1);buffer_add_ch(&msg,')');
  displayProgress.emit(&msg);
  buffer_free(&msg);

  is_connected = true;
  return true;
}

bool TLSConnection::doClose() {
  if (is_connected)
    gnutls_bye (state, GNUTLS_SHUT_RDWR);
  gnutls_certificate_free_credentials (xcred);
  gnutls_deinit (state);
  bool was_connected = is_connected;
  is_connected = false;
  return was_connected;
}

bool TLSConnection::getVersion(buffer_t* dst) {
  if (!dst)
    return true;
  buffer_add_str(dst,"gnutls ",7);
  buffer_add_str(dst,LIBGNUTLS_VERSION,-1);
  return true;
}
