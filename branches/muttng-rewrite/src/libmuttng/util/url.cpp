/** @ingroup libmuttng_util */
/**
 * @file libmuttng/util/url.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @author Copyright (C) 2000 Thomas Roessler <roessler@does-not-exist.org>
 * @brief Implementation: URL handling
 */
#include <ctype.h>
#include <string.h>
#include <iostream>

#include "core/buffer.h"
#include "core/str.h"
#include "core/mem.h"
#include "core/intl.h"

#include "url.h"

static struct {
  urlproto_t proto;
  const char* str;
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

static int hexval (unsigned char c) {
  unsigned char d = 0;
  if (c >= '0' && c <= '9')
    return (c - '0');
  d = tolower (c);
  if (c >= 'a' && c <= 'f')
    return (10 + (c - 'a'));
  return (-1);
}

static bool url_pct_decode (char* url, int* chars) {
  char* d;

  if (!url)
    return (true);

  for (d = url; *url; url++) {
    if (*url == '%') {
      /*
       * be sure to block %00 aka \0, there were some bugtraq
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

urlproto_t url_check_proto (const char* s, bool* secure, buffer_t* error) {
  char sbuf[32];
  char *t = NULL;
  int i = 0;

  if (!s || !(t = strchr (s, ':')) || (t - s) + 1 >= (int) sizeof (sbuf)) {
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

/* parse_userhost: fill in components of url with info from src. Note
 *   these are pointers into src, which is altered with '\0's. Port of 0
 *   means no port given. */
bool parse_userhost (url_t * url, char *src, buffer_t* error) {
  char *t;
  char *p;
  char *path;
  char *err;
  int chars = 0;

  url->username = NULL;
  url->password = NULL;
  url->host = NULL;
  url->port = 0;

  if (str_ncmp (src, "//", 2))
    return (str_dup (src));

  src += 2;

  if ((path = strchr (src, '/')))
    *path++ = '\0';

  if ((t = strrchr (src, '@'))) {
    *t++ = '\0';
    if ((p = strchr (src, ':'))) {
      *p = '\0';
      url->password = str_dup (p + 1);
      if (!url_pct_decode (url->password, &chars)) {
        err = url->password;
        goto error;
      }
    }
    url->username = str_dup (src);
    if (!url_pct_decode (url->username, &chars)) {
      err = url->username;
      goto error;
    }
  }
  else
    t = src;

  if ((p = strchr (t, ':'))) {
    *p++ = '\0';
    url->port = atoi (p);
  }
  else
    url->port = 0;

  if (t && *t)
    url->host = strdup (t);
  if (!url_pct_decode (url->host, &chars)) {
    err = url->host;
    goto error;
  }
  if (path && *path)
    url->path = strdup (path);
  return (true);

error:
  if (error) {
    buffer_add_str (error, _("decoding error in: "), -1);
    buffer_add_str (error, err+chars, -1);
  }
  url_free (url);
  return (false);
}

void url_free (url_t* url) {
  mem_free (&url->username);
  mem_free (&url->password);
  mem_free (&url->host);
  mem_free (&url->path);
}

/* url_parse_url: Fill in url_t. char* elements are pointers into src,
 *   which is modified by this call (duplicate it first if you need to). */
url_t* url_from_string (const char* url, buffer_t* error) {
  char* tmp = NULL, *buf = NULL;
  url_t* ret = NULL;
  urlproto_t proto = P_LAST;
  bool secure = false;
  int chars = 0;

  if ((proto = url_check_proto (url, &secure, error)) == P_LAST)
    return (NULL);

  ret = new url_t (proto, secure);
  buf = str_dup (url);
  tmp = strchr (buf, ':') + 1;

  if (!parse_userhost (ret, tmp, error))
    goto error;
  if (!url_pct_decode (ret->path, &chars)) {
    if (error) {
      buffer_add_str (error, _("decoding error in: "), -1);
      buffer_add_str (error, ret->path+chars, -1);
    }
    goto error;
  }

  if (proto == P_FILE && (ret->host || ret->username || ret->password)) {
    if (error)
      buffer_add_str (error, _("no remote files supported "
                               "for \"file\" protocol"), -1);
    goto error;
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
  buffer_add_ch (dst, '/');
  buffer_add_str (dst, url->path, -1);
}

