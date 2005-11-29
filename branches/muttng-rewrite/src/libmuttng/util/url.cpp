/** @ingroup libmuttng_util */
/**
 * @file libmuttng/util/url.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @author Copyright (C) 2000 Thomas Roessler <roessler@does-not-exist.org>
 * @brief Implementation: URL handling
 */
#include <ctype.h>
#include <iostream>
#include <cstring>

#include "core/buffer.h"
#include "core/str.h"
#include "core/mem.h"
#include "core/intl.h"

#include "url.h"

/**
 * Table for mapping protos to strings we accept.
 */
static struct {
  /** protocol */
  urlproto_t proto;
  /** string */
  const char* str;
  /** whether proto uses SSL */
  bool secure;
} Protos[] = {
  { P_FILE,     "file",         0 },
#if LIBMUTTNG_IMAP
  { P_IMAP,     "imap",         0 },
  { P_IMAP,     "imaps",        1 },
#endif
#if LIBMUTTNG_NNTP
  { P_NNTP,     "nntp",         0 },
  { P_NNTP,     "news",         0 },
  { P_NNTP,     "nntps",        1 },
  { P_NNTP,     "snews",        1 },
#endif
#if LIBMUTTNG_POP3
  { P_POP3,     "pop",          0 },
  { P_POP3,     "pops",         1 },
#endif
#if LIBMUTTNG_SMTP
  { P_SMTP,     "smtp",         0 },
  { P_SMTP,     "ismtps",       1 },
#endif
  { P_LAST,     NULL,           0 }
};

/**
 * Interpret character as hex and return decimal value.
 * @param c Character.
 * @return
 *   - decimal value upon success
 *   - @c -1 upon error
 */
static int hexval (unsigned char c) {
  unsigned char d = 0;
  if (c >= '0' && c <= '9')
    return (c - '0');
  d = tolower (c);
  if (d >= 'a' && d <= 'f')
    return (10 + (d - 'a'));
  return (-1);
}

/**
 * Decode an URL.
 * This will fail on incomplete and the magic @c %00 sequence.
 * @param url URL to decode.
 * @param chars If given a pointer to @c int, this stores how many
 *              characters of input have been processed to point
 *              out at which place an error occured.
 * @return Success.
 */
static bool url_decode (char* url, int* chars) {
  char* d;

  if (!url)
    return (true);

  if (chars)
    *chars = 0;

  for (d = url; *url; url++) {
    if (*url == '%') {
      /**
       * Be sure to block %00 aka \\0, there were some bugtraq
       * posts so be strict ;-)
       */
      if (url[1] && url[2] && strncmp (&url[1], "00", 2) != 0 &&
          hexval ((unsigned char) url[1]) >= 0 &&
          hexval ((unsigned char) url[2]) >= 0) {
        *d++ = (hexval ((unsigned char) url[1]) << 4) |
               (hexval ((unsigned char) url[2]));
        url += 2;
      } else
        return (false);
    } else if (*url == '+')
      *d++ = ' ';
    else
      *d++ = *url;
    if (chars)
      (*chars)++;
  }
  *d = '\0';
  return (true);
}

/**
 * For an URL string, check and return protocol.
 * @param s URL String.
 * @param secure Pointer to storage to indicate whether SSL is used or not.
 * @param error Error buffer for error message.
 * @return
 *      - protocol upon success
 *      - @c P_LAST otherwise
 */
static urlproto_t url_get_proto (const char* s, bool* secure, buffer_t* error) {
  char sbuf[32];        /* 32 should suffice for any ^[a-z]: string */
  char *t = NULL;
  int i = 0;

  if (!s || !(t = (char*) strchr (s, ':')) ||
      (t - s) + 1 >= (int) sizeof (sbuf)) {
    if (error)
      buffer_add_str (error, _("internal error"), -1);
    return P_LAST;
  }

  strfcpy (sbuf, s, t - s + 1);
  for (t = sbuf; *t; t++)
    *t = tolower (*t);

  for (i = 0; Protos[i].str; i++)
    if (str_eq (Protos[i].str, sbuf)) {
      *secure = Protos[i].secure;
      return (Protos[i].proto);
    }

  if (error) {
    buffer_add_str (error, _("unsupported protocol: "), -1);
    buffer_add_str (error, sbuf, -1);
  }
  return (P_LAST);
}

/**
 * Parse URL string without protocol into url_t.
 * This may fail for several reasons:
 *   - incomplete/invalid encoded sequence(s)
 *   - empty fields (like username) though format implies an optional field
 * @param url Destination.
 * @param src Source string.
 * @param error Error buffer.
 * @return Success.
 */
static bool parse_userhost (url_t* url, char* src, buffer_t* error) {
  char *t;
  char *p;
  char *path;
  char *err;
  int chars = 0;
  size_t pathlen = 0;

  url->username = NULL;
  url->password = NULL;
  url->host = NULL;
  url->port = 0;
  url->path = NULL;

  if (str_ncmp (src, "//", 2))
    return (str_dup (src));

  src += 2;

  if ((path = strchr (src, '/')))
    *path++ = '\0';

  if ((t = strrchr (src, '@'))) {
    *t++ = '\0';
    if ((p = strchr (src, ':'))) {
      *p = '\0';
      if (str_len (p+1) == 0)
        goto field_error;
      url->password = str_dup (p + 1);
      if (!url_decode (url->password, &chars)) {
        err = url->password;
        goto decode_error;
      }
    }
    if (str_len (src) == 0)
      goto field_error;
    url->username = str_dup (src);
    if (!url_decode (url->username, &chars)) {
      err = url->username;
      goto decode_error;
    }
  }
  else
    t = src;

  if ((p = strchr (t, ':'))) {
    *p++ = '\0';
    if (str_len (p) == 0)
      goto field_error;
    url->port = atoi (p);
  }
  else
    url->port = 0;

  if (t && *t)
    url->host = str_dup (t);
  if (!url_decode (url->host, &chars)) {
    err = url->host;
    goto decode_error;
  }

  /* always fully qualify path with leading '/' */
  pathlen = str_len (path);
  url->path = (char*) mem_malloc (pathlen+2);
  url->path[0] = '/';
  url->path[pathlen+1] = '\0';
  if (pathlen)
    memcpy (url->path+1, path, pathlen);
  if (!url_decode (url->path, &chars)) {
    err = url->path;
    goto decode_error;
  }

  return (true);

decode_error:
  if (error) {
    buffer_add_str (error, _("decoding error in: "), -1);
    buffer_add_str (error, err+chars, -1);
  }
  url_free (url);
  return (false);

field_error:
  if (error)
    buffer_add_str (error, _("empty field in URL"), -1);
  url_free (url);
  return (false);
}

void url_free (url_t* url) {
  mem_free (&url->username);
  mem_free (&url->password);
  mem_free (&url->host);
  mem_free (&url->path);
}

url_t* url_from_string (const char* url, buffer_t* error) {
  char* tmp = NULL, *buf = NULL;
  url_t* ret = NULL;
  urlproto_t proto = P_LAST;
  bool secure = false;

  if ((proto = url_get_proto (url, &secure, error)) == P_LAST)
    return (NULL);

  ret = new url_t (proto, secure);
  buf = str_dup (url);
  tmp = strchr (buf, ':') + 1;

  if (!parse_userhost (ret, tmp, error))
    goto error;

  /* treat file:// specially */
  if (ret->host && *ret->host) {
    if (proto == P_FILE) {
      if (error)
        buffer_add_str (error, _("no remote files supported "
                                 "for \"file\" protocol"), -1);
      goto error;
    }
  } else {
    if (proto != P_FILE) {
      if (error)
        buffer_add_str (error, _("no host given for protocol"), -1);
      goto error;
    }
  }

  mem_free (&buf);
  return (ret);

error:
  url_free (ret);
  delete (ret);
  mem_free (&buf);
  return (NULL);
}

void url_to_string (url_t* url, buffer_t* dst, bool pwd) {
  int i = 0;

  if (!url || !dst)
    return;

  for (i = 0; Protos[i].str; i++)
    if (Protos[i].proto == url->proto &&
        Protos[i].secure == url->secure) {
      buffer_add_str (dst, Protos[i].str, -1);
      buffer_add_str (dst, "://", 3);
      break;
    }
  if (url->username) {
    buffer_add_str (dst, url->username, -1);
    if (url->password && pwd) {
      buffer_add_ch (dst, ':');
      buffer_add_str (dst, url->password, -1);
    }
    buffer_add_ch (dst, '@');
  }
  buffer_add_str (dst, url->host, -1);
  /* XXX default port */
  buffer_add_str (dst, url->path, -1);
}

bool url_eq (url_t* url1, url_t* url2) {
  if (!url1 || !url2)
    return (!url1 && !url2);
  return (str_eq (url1->username, url2->username) &&
          str_eq (url1->password, url2->password) &&
          str_eq (url1->host, url2->host) &&
          str_eq (url1->path, url2->path) &&
          url1->port == url2->port &&
          url1->secure == url2->secure &&
          url1->proto == url2->proto);
}
