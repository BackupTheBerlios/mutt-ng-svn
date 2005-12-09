/**
 * @ingroup libmuttng_util
 * @addtogroup libmuttng_url URL handling
 * @{
 * For implementation details and an example, please see
 * @ref sect_devguide-libmuttng-url.
 */
/**
 * @file libmuttng/util/url.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: URL handling
 */
#ifndef LIBMUTTNG_UTIL_URL_H
#define LIBMUTTNG_UTIL_URL_H

#include "core/buffer.h"

#include "libmuttng/libmuttng.h"
#include "libmuttng/libmuttng_features.h"

/** all valid protocols */
typedef enum {
  /** @c file:// URLs */
  P_FILE = 0,
#if LIBMUTTNG_IMAP
  /** @c imap[s]:// URL */
  P_IMAP,
#endif
#if LIBMUTTNG_NNTP
  /** @c nntp[s]:// URL */
  P_NNTP,
#endif
#if LIBMUTTNG_POP3
  /** @c pop[s]:// URL */
  P_POP3,
#endif
#if LIBMUTTNG_SMTP
  /** @c smtp[s]:// URL */
  P_SMTP,
#endif
  P_LAST
} urlproto_t;

/** URL representation */
typedef struct url_t {
  /** optional: username */
  char* username;
  /** optional: password */
  char* password;
  /** optional: host */
  char* host;
  /** optional: port */
  unsigned short port;
  /** default port; don't touch */
  unsigned short defport;
  /** optional: whether we're using SSL */
  bool secure;
  /** mandatory: path */
  char* path;
  /** mandatory: protocol */
  urlproto_t proto;
#ifndef LIBMUTTNG_TEST
  /**
   * Init new url_t structure.
   * @param proto_ protocol.
   * @param secure_ secure.
   * @param defport_ default port.
   */
  url_t(urlproto_t proto_, bool secure_, unsigned short defport_) {
    username = NULL;
    password = NULL;
    host = NULL;
    port = 0;
    defport = defport_;
    secure = secure_;
    proto = proto_; 
  }
#endif
} url_t;

/**
 * Parse string into url_t.
 * @param url URL as string.
 * @param error Buffer where to put error messages.
 * @return url_t structure or @c NULL for failure.
 * @test url_tests::test_invalid_proto().
 * @test url_tests::test_parse().
 * @test url_tests::test_decode().
 */
url_t* url_from_string (const char* url, buffer_t* error);

/**
 * Convert url_t back into string.
 * @param url URL to convert.
 * @param dst Destination buffer.
 * @param pwd Whether to print passwords.
 */
void url_to_string (url_t* url, buffer_t* dst, bool pwd);

/**
 * Free all memory internally used to url.
 * @b NOTE: this does not free url itself.
 * @param url URL.
 */
void url_free (url_t* url);

/**
 * Test whether to URLs are equal.
 * @param url1 1st url.
 * @param url2 2nd url.
 * @return Yes/No.
 * @test url_tests::test_parse().
 */
bool url_eq (url_t* url1, url_t* url2);

/**
 * For a given protocoll, obtain default port.
 * @param proto Protocoll.
 * @param secure Whether connection is secure or not.
 * @return port
 */
unsigned short url_defport (urlproto_t proto, bool secure);

#endif /* !LIBMUTTNG_MESSGAGE_BODY_H */

/** @} */
