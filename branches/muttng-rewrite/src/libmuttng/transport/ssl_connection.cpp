/** @ingroup libmuttng_transport */
/**
 * @file libmuttng/transport/ssl_connection.cpp
 * @brief Implementation: OpenSSL-based TCP connection
 *
 * This file is published under the GNU General Public License.
 */
#include "ssl_connection.h"

#include "core/mem.h"
#include "core/intl.h"
#include "core/str.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>

#include <cstring>
#include <vector>

#include <openssl/opensslv.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#include "libmuttng/config/config_manager.h"

#if OPENSSL_VERSION_NUMBER >= 0x00904000L
#define READ_X509_KEY(fp, key)  PEM_read_X509(fp, key, NULL, NULL)
#else
#define READ_X509_KEY(fp, key)  PEM_read_X509(fp, key, NULL)
#endif

/* Just in case OpenSSL doesn't define DEVRANDOM */
#ifndef DEVRANDOM
#define DEVRANDOM "/dev/urandom"
#endif

/* This is ugly, but as RAND_status came in on OpenSSL version 0.9.5
 * and the code has to support older versions too, this is seemed to
 * be cleaner way compared to having even uglier #ifdefs all around.
 */
#ifdef HAVE_RAND_STATUS
#define HAVE_ENTROPY()  (RAND_status() == 1)
#else
/** how many entropy bytes we managed to gather manually */
static int entropy_byte_count = 0;

/* OpenSSL fills the entropy pool from /dev/urandom if it exists */
#define HAVE_ENTROPY()  (!access(DEVRANDOM, R_OK) || entropy_byte_count >= 16)
#endif

/** whether OpenSSL library is initialized */
static bool did_init = false;
/** cache of certificates for session */
static std::vector<X509*> Certs;
/** storage for @ref option_ssl_entropy_file */
static char* SSLEntropyFile = NULL;
/** storage for @ref option_ssl_usesystemcerts */
static bool UseSysCerts = false;

SSLConnection::SSLConnection(url_t* url_) :
  Connection(url_),ctx(NULL),ssl(NULL),cert(NULL) {
  if (!did_init) init();
}

SSLConnection::~SSLConnection() {}

int SSLConnection::add_entropy (const char *file) {
  struct stat st;
  int n = -1;
  buffer_t msg;
  buffer_init(&msg);

  if (!file)
    return 0;

  if (stat (file, &st) == -1)
    return errno == ENOENT ? 0 : -1;

  buffer_add_str(&msg,_("Filling entropy pool: '"),-1);
  buffer_add_str(&msg,file,-1);
  buffer_add_str(&msg,_("'..."),-1);
  displayProgress.emit(&msg);

  /* check that the file permissions are secure */
  if (st.st_uid != getuid () ||
      ((st.st_mode & (S_IWGRP | S_IRGRP)) != 0) ||
      ((st.st_mode & (S_IWOTH | S_IROTH)) != 0)) {
    buffer_shrink(&msg,0);
    buffer_add_str(&msg,_("Entropy file '"),-1);
    buffer_add_str(&msg,file,-1);
    buffer_add_str(&msg,_("' has insecure permissions."),-1);
    displayError.emit(&msg);
    buffer_free(&msg);
    return -1;
  }

#ifdef HAVE_RAND_EGD
  n = RAND_egd (file);
#endif
  if (n <= 0)
    n = RAND_load_file (file, -1);

#ifndef HAVE_RAND_STATUS
  if (n > 0)
    entropy_byte_count += n;
#endif
  buffer_free(&msg);
  return n;
}

void SSLConnection::init () {

  did_init = false;

  buffer_t path;
  buffer_init(&path);
  buffer_grow(&path,_POSIX_PATH_MAX+1);

  if (!HAVE_ENTROPY ()) {
    /* load entropy from files */
    if (SSLEntropyFile)
      add_entropy (SSLEntropyFile);
    add_entropy (RAND_file_name (path.str,path.size));

    /* load entropy from egd sockets */
#ifdef HAVE_RAND_EGD
    add_entropy (getenv ("EGDSOCKET"));
    buffer_shrink(&path,0);
    buffer_add_str(&path,NONULL(Homedir),-1);
    buffer_add_str(&path,"/.entropy",9);
    add_entropy (path.str);
    add_entropy ("/tmp/entropy");
#endif

    /* shuffle $RANDFILE (or ~/.rnd if unset) */
    RAND_write_file (RAND_file_name (path.str,path.size));
    if (!HAVE_ENTROPY ()) {
      buffer_t msg; buffer_init(&msg);
      buffer_add_str(&msg,_("Failed to find enough entropy on your system"),-1);
      displayError.emit(&msg);
      buffer_free(&msg);
      buffer_free(&path);
      return;
    }
  }

  /*
   * I don't think you can do this just before reading the error.
   * The call itself might clobber the last SSL error.
   */
  SSL_load_error_strings ();
  SSL_library_init ();
  did_init = true;
  buffer_free(&path);
}

void SSLConnection::reg() {
  Option* opt = ConfigManager::regOption(new StringOption("ssl_entropy_file","",&SSLEntropyFile));
  ConfigManager::regOption(new SynOption("entropy_file",opt));
  ConfigManager::regOption(new BoolOption("ssl_usesystemcerts","true",&UseSysCerts));
  ConfigManager::regFeature("ssl");
  ConfigManager::regFeature("openssl");
}

void SSLConnection::dereg() {
  while (Certs.size()!=0) {
    X509* tmp = Certs.back();
    Certs.pop_back();
    X509_free(tmp);
  }
}

int SSLConnection::doRead(buffer_t * buf, unsigned int len) {
  int read_len;

  buffer_grow(buf,len);
  buffer_shrink(buf,0);
  read_len = SSL_read(ssl,buf->str,len);

  switch (read_len) {
    case -1:
      is_connected = false;
      return -1;
    case 0:
      is_connected = false;
    default:
      break;
  }
  return read_len;
}

int SSLConnection::doWrite(buffer_t * buf) {
  if (!buf) return -1;
  int rc = SSL_write(ssl,buf->str,buf->len);
  if (rc<0) is_connected = false;
  return rc;
}

void SSLConnection::getClientCert() {
  if (SSLClientCert) {
    /* XXX
    SSL_CTX_set_default_passwd_cb_userdata (ssldata->ctx, &conn->account);
    SSL_CTX_set_default_passwd_cb (ssldata->ctx, ssl_passwd_cb);
     */
    SSL_CTX_use_certificate_file(ctx,SSLClientCert,SSL_FILETYPE_PEM);
    SSL_CTX_use_PrivateKey_file(ctx,SSLClientCert,SSL_FILETYPE_PEM);
  }
}

bool SSLConnection::checkCertCache() {
  unsigned char peermd[EVP_MAX_MD_SIZE];
  unsigned int peermdlen;
  size_t i;

  if (!X509_digest (cert, EVP_sha1(), peermd, &peermdlen)) 
    return false;

  for (i=0; i<Certs.size(); i++) {
    if (X509_cmp(Certs[i],peermd,peermdlen)) {
      DEBUGPRINT(D_SOCKET,("found cached cert at %d",i));
      return true;
    }
  }
  return false;
}

bool SSLConnection::checkCertSigner() {
  X509_STORE_CTX xsc;
  X509_STORE *c;
  bool pass = false;

  if (!(c = X509_STORE_new())) return false;

  if (UseSysCerts) {
    if (!X509_STORE_set_default_paths (c)) {
      DEBUGPRINT(D_SOCKET,(("X509_STORE_set_default_paths failed")));
    } else
      pass = true;
  }

  if (!X509_STORE_load_locations (c, SSLCertFile, NULL)) {
    DEBUGPRINT(D_SOCKET,("X509_STORE_load_locations_failed"));
  } else
    pass = true;

  if (!pass) {
    /* nothing to do */
    X509_STORE_free (c);
    return false;
  }

  X509_STORE_CTX_init (&xsc, c, cert, NULL);
  pass = (X509_verify_cert (&xsc) > 0);
  X509_STORE_CTX_cleanup (&xsc);
  X509_STORE_free (c);

  return pass;
}

bool SSLConnection::X509_cmp (X509 *c, unsigned char *peermd,
                             unsigned int peermdlen) {
  unsigned char md[EVP_MAX_MD_SIZE];
  unsigned int mdlen;

  /*
   * Avoid CPU-intensive digest calculation if the certificates are not
   * even remotely equal.
   */
  if (X509_subject_name_cmp (cert, c) != 0 ||
      X509_issuer_name_cmp (cert, c) != 0)
    return false;

  if (!X509_digest (c, EVP_sha1(), md, &mdlen) || peermdlen != mdlen)
    return false;

  if (memcmp(peermd, md, mdlen) != 0)
    return false;

  return false;
}

bool SSLConnection::checkCertDigest() {
  unsigned char peermd[EVP_MAX_MD_SIZE];
  unsigned int peermdlen;
  X509 *c = NULL; 
  bool pass = false;
  FILE *fp;
  buffer_t msg;

  buffer_init(&msg);
  /* expiration check */
  if (X509_cmp_current_time (X509_get_notBefore (cert)) >= 0) {
    buffer_shrink(&msg,0);
    buffer_add_str(&msg,_("Server certificate is not yet valid."),-1);
    displayError.emit(&msg);
    buffer_free(&msg);
    return 0;
  }
  if (X509_cmp_current_time (X509_get_notAfter (cert)) <= 0) { 
    buffer_shrink(&msg,0);
    buffer_add_str(&msg,_("Server certificate has expired."),-1);
    displayError.emit(&msg);
    buffer_free(&msg);
    return 0;
  }
  buffer_free(&msg);

  if ((fp = fopen (SSLCertFile, "rt")) == NULL)
    return false;

  if (!X509_digest (cert, EVP_sha1 (), peermd, &peermdlen)) {
    fclose (fp);
    return false;
  }

  while ((c = READ_X509_KEY (fp, &c)) != NULL) { 
    pass = X509_cmp (c, peermd, peermdlen);
    if (pass)
      break;
  }

  X509_free(c);
  fclose (fp);

  return pass;
}

char* SSLConnection::X509_get_part (char *line, const char *ndx) {
  static char ret[128];
  char *c, *c2;

  strfcpy (ret, _("Unknown"), sizeof (ret)); 

  c = strstr (line, ndx);
  if (c) {
    c += str_len (ndx);
    c2 = strchr (c, '/');
    if (c2) 
      *c2 = '\0'; 
    strfcpy (ret, c, sizeof (ret)); 
    if (c2) 
      *c2 = '/';
  }

  return ret;
}

void SSLConnection::X509_fingerprint (char *s, int l, X509 * cert) {
  unsigned char md[EVP_MAX_MD_SIZE];
  unsigned int n;
  int j;

  if (!X509_digest (cert, EVP_md5 (), md, &n)) {
    snprintf (s, l, "%s", _("[unable to calculate]"));
  }
  else {
    for (j = 0; j < (int) n; j++) {
      char ch[8];

      snprintf (ch, 8, "%02X%s", md[j], (j % 2 ? " " : ""));
      str_cat (s, l, ch);
    }
  }
}

char* SSLConnection::asn1time_to_string (ASN1_UTCTIME * tm) {
  static char buf[64];
  BIO *bio;

  strfcpy (buf, _("[invalid date]"), sizeof (buf));

  bio = BIO_new (BIO_s_mem ());
  if (bio) {
    if (ASN1_TIME_print (bio, tm))
      (void) BIO_read (bio, buf, sizeof (buf));
    BIO_free (bio);
  }

  return buf;
}

void SSLConnection::fillCertInfo(certinfo_t* dst, X509_NAME* (*getinfo)(X509*)) {
  static const char* part[] = { "/CN=", "/Email=", "/O=", "/OU=", "/L=", "/ST=", "/C=" };
  char* name = NULL, *c;
  char** str = NULL;
  unsigned short i;
  char buf[128];

  name = X509_NAME_oneline(getinfo(cert),buf,sizeof(buf));
  for (i=0; i<7; i++) {
    c = X509_get_part(name,part[i]);
    switch(i) {
    case 0: str = &dst->name; break;
    case 1: str = &dst->contact; break;
    case 2: str = &dst->org; break;
    case 3: str = &dst->unit; break;
    case 4: str = &dst->location; break;
    case 5: str = &dst->state; break;
    case 6: str = &dst->country; break;
    default: break;
    }
    if (str) *str = str_dup(c);
  }
}

bool SSLConnection::checkCert() {
  /* check session cache first */
  if (checkCertCache()) {
    DEBUGPRINT(D_SOCKET,("using cached cert"));
    return true;
  }

  if (checkCertSigner()) {
    DEBUGPRINT(D_SOCKET,("signer check passed"));
    Certs.push_back(cert);
    return true;
  }

  /* automatic check from user's database */
  if (SSLCertFile && checkCertDigest()) {
    DEBUGPRINT(D_SOCKET,("digest check passed"));
    Certs.push_back(cert);
    return true;
  }

  if (sigCheckCertificate.getHandlers()==0) {
    buffer_t error;
    buffer_init(&error);
    buffer_add_str(&error,_("Library programming error: no handlers for certificate check. Report this!"),-1);
    displayError.emit(&error);
    buffer_free(&error);
    return false;
  }

  certinfo_t owner = certinfo_t(), issuer = certinfo_t();
  char fp[128], *from, *to;
  certcheck_t result = CERT_REJECT;

  fillCertInfo(&owner,X509_get_subject_name);
  fillCertInfo(&issuer,X509_get_issuer_name);
  X509_fingerprint(fp,sizeof(fp),cert);
  from = str_dup(asn1time_to_string(X509_get_notBefore(cert)));
  to = str_dup(asn1time_to_string(X509_get_notAfter(cert)));

  bool rc = sigCheckCertificate.emit(&owner,&issuer,fp,from,to,&result);

  freeCertInfo(&owner);
  freeCertInfo(&issuer);
  mem_free(&from);
  mem_free(&to);

  return rc;
}

bool SSLConnection::negotiate () {
  int err;
  const char *errmsg;
  buffer_t msg;

  buffer_init(&msg);

#if OPENSSL_VERSION_NUMBER >= 0x00906000L
  /* This only exists in 0.9.6 and above. Without it we may get interrupted
   *   reads or writes. Bummer. */
  SSL_set_mode (ssl, SSL_MODE_AUTO_RETRY);
#endif

  if ((err = SSL_connect (ssl)) != 1) {
    switch (SSL_get_error (ssl, err)) {
    case SSL_ERROR_SYSCALL:
      errmsg = _("I/O error");
      break;
    case SSL_ERROR_SSL:
      errmsg = ERR_error_string (ERR_get_error (), NULL);
      break;
    default:
      errmsg = _("unknown error");
    }
    buffer_shrink(&msg,0);
    buffer_add_str(&msg,(_("OpenSSL's connect() failed: ")),-1);
    buffer_add_str(&msg,errmsg,-1);
    displayError.emit(&msg);
    buffer_free(&msg);
    return false;
  }

  if (!(cert = SSL_get_peer_certificate (ssl))) {
    buffer_shrink(&msg,0);
    buffer_add_str(&msg,_("Unable to get certificate from peer"),-1);
    displayError.emit(&msg);
    buffer_free(&msg);
    return false;
  }

  if (!checkCert())
    return false;

  return true;
}

bool SSLConnection::doOpen() {
  if (!did_init) return false;

  ctx = SSL_CTX_new (SSLv23_client_method ());

  /* disable SSL protocols as needed */
  if (!UseTLS1) {
    SSL_CTX_set_options (ctx, SSL_OP_NO_TLSv1);
  }
  if (!UseSSL3) {
    SSL_CTX_set_options (ctx, SSL_OP_NO_SSLv3);
  }
  if (!UseTLS1 && !UseSSL3)
    return false;

  getClientCert ();

  ssl = SSL_new (ctx);
  SSL_set_fd (ssl,fd);

  if (!negotiate ())
    return false;

  int maxbits;
  ssf = SSL_CIPHER_get_bits (SSL_get_current_cipher (ssl),&maxbits);

  buffer_t msg;
  buffer_init(&msg);
  buffer_add_str(&msg,_("SSL/TLS connection using "),-1);
  buffer_add_str(&msg,SSL_get_cipher_version(ssl),-1);
  buffer_add_str(&msg," (",2);
  buffer_add_str(&msg,SSL_get_cipher_name(ssl),-1);
  buffer_add_ch(&msg,')');
  displayProgress.emit(&msg);
  buffer_free(&msg);

  return true;
}

bool SSLConnection::doClose() {
  if (ssl) SSL_free(ssl);
  if (ctx) SSL_CTX_free(ctx);
  return true;
}

static inline unsigned char pl(unsigned short a) {
  switch(a) {
  case 0: return ' ';
  default: return 'a'+a-1;
  }
  return ' ';
}

bool SSLConnection::getVersion (buffer_t* dst) {
  if (!dst)
    return true;
  buffer_add_str(dst,"openssl ",8);
  buffer_add_snum(dst,(OPENSSL_VERSION_NUMBER>>28)&0xf,-1);buffer_add_ch(dst,'.');
  buffer_add_snum(dst,(OPENSSL_VERSION_NUMBER>>20)&0xff,-1);buffer_add_ch(dst,'.');
  buffer_add_snum(dst,(OPENSSL_VERSION_NUMBER>>12)&0xff,-1);
  buffer_add_ch(dst,pl(((OPENSSL_VERSION_NUMBER>>4)&0xff)%27));
  return true;
}
