/** @ingroup libmuttng_transport */
/**
 * @file libmuttng/transport/ssl_connection.h
 * @brief Interface: OpenSSL-based TCP connection
 */

#ifndef LIBMUTTNG_TRANSPORT_SSL_CONNECTION__H
#define LIBMUTTNG_TRANSPORT_SSL_CONNECTION__H

#include "connection.h"

#include <openssl/ssl.h>
#include <openssl/x509.h>

/**
 * Abstract TCP connection.
 */
class SSLConnection : public Connection {
  public:
    /** @copydoc Connection::Connection(). */
    SSLConnection(url_t* url_);
    ~SSLConnection();
    int doRead(buffer_t * buf, unsigned int len);
    int doWrite(buffer_t * buf);
    bool doOpen();
    bool doClose();
    /** @copydoc Connection::getVersion(). */
    static bool getVersion(buffer_t* dst);
    /** register SSL specific variables */
    static void reg();
    /** deregister internal items */
    static void dereg();
  private:
    /** SSL context */
    SSL_CTX* ctx;
    /** SSL state */
    SSL* ssl;
    /** Peer certificate */
    X509* cert;
    /** Get client's certificate */
    void getClientCert();
    /**
     * Negotiate SSL options and check certificate.
     * @return Success.
     */
    bool negotiate();
    /**
     * Check certificate.
     * @return Valid/acceptable or not.
     */
    bool checkCert();
    /**
     * Check certificate by checking internal cache.
     * @return Success, ie certificate is cached for session.
     */
    bool checkCertCache();
    /**
     * Check certificate by checking signer.
     * @return Success.
     */
    bool checkCertSigner();
    /**
     * Check certificate by digest, ie user's db.
     * @return Success.
     */
    bool checkCertDigest();
    /**
     * See if passed certificate is a candiate for being same as ours.
     * @param c Cert to check.
     * @param peermd ?
     * @param peermdlen ?
     * @return Equal or not.
     */
    bool X509_cmp (X509 *c, unsigned char *peermd, unsigned int peermdlen);
    /**
     * Manually feed OpenSSL with entropy.
     * @param file From which file to feed.
     * @return How many entropy bytes we gathered manually.
     */
    int add_entropy(const char* file);
    /** Initialize OpenSSL library */
    void init();
    /**
     * Fill certinfo_t structure.
     * @param dst Destination.
     */
    void fillCertInfo(certinfo_t* dst, X509_NAME* (*get)(X509*));
    char* X509_get_part (char *line, const char *ndx);
    char* asn1time_to_string (ASN1_UTCTIME * tm);
    void X509_fingerprint (char *s, int l, X509 * cert);
};

#endif
