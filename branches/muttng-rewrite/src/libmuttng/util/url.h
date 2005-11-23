/** @ingroup libmuttng_util */
/**
 * @file libmuttng/util/url.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: URL handling
 */
#ifndef LIBMUTTNG_UTIL_URL_H
#define LIBMUTTNG_UTIL_URL_H

#include "core/buffer.h"

#include "../libmuttng.h"
#include "../features.h"

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
  int port;
  /** optional: whether we're using SSL */
  bool secure;
  /** mandatory: path */
  char* path;
  /** mandatory: protocol */
  urlproto_t proto;
  /**
   * Init new url_t structure.
   * @param proto_ protocol.
   * @param secure_ secure.
   */
  url_t(urlproto_t proto_, bool secure_) {
    username = NULL;
    password = NULL;
    host = NULL;
    port = 0;
    secure = secure_;
    proto = proto_; 
  }
} url_t;

/**
 * Parse string into url_t.
 * @param url URL as string.
 * @return url_t structure or @c NULL for failure.
 */
url_t* url_from_string (const char* url, buffer_t* error);

/**
 * Convert url_t back into string.
 * As passwords will be internally used for authentication only, they're
 * never added to destination buffer.
 * @param url URL to convert.
 * @param dst Destination buffer.
 */
void url_to_string (url_t* url, buffer_t* dst, bool secure);

void url_free (url_t* url);

#endif /* !LIBMUTTNG_MESSGAGE_BODY_H */
