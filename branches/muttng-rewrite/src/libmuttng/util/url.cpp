/** @ingroup libmuttng_url */
/**
 * @file libmuttng/util/url.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @author Copyright (C) 2000 Thomas Roessler <roessler@does-not-exist.org>
 * @brief Implementation: URL handling
 */
#include <ctype.h>
#include <cstring>

#include <string.h>

#include "core/buffer.h"
#include "core/str.h"
#include "core/mem.h"
#include "core/intl.h"
#include "core/qp.h"
#include "core/net.h"

#include "url.h"

/**
 * QP-decode an URL. This just wraps buffer_decode_qp().
 * @param str Pointer to string.
 * @param chars Pointer for error storage of buffer_decode_qp().
 * @return Success.
 */
static bool url_decode (char** str, size_t* chars) {
  buffer_t tmp1, tmp2;
  int rc = 1;

  buffer_init(&tmp1);
  buffer_init(&tmp2);

  buffer_add_str(&tmp1,*str,-1);
  if ((rc=buffer_decode_qp(&tmp2,&tmp1,'%',chars))) {
    mem_free(str);
    *str = tmp2.str;
  } else
    buffer_free(&tmp2);
  buffer_free(&tmp1);
  return rc;
}

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
  /** default port */
  unsigned int defport;
} Protos[] = {
  { P_FILE,     "file",         0,      0 },
#if LIBMUTTNG_IMAP
  { P_IMAP,     "imap",         0,      143 },
  { P_IMAP,     "imaps",        1,      993 },
#endif
#if LIBMUTTNG_NNTP
  { P_NNTP,     "nntp",         0,      119 },
  { P_NNTP,     "news",         0,      119 },
  { P_NNTP,     "nntps",        1,      563 },
  { P_NNTP,     "snews",        1,      563 },
#endif
#if LIBMUTTNG_POP3
  { P_POP3,     "pop",          0,      110 },
  { P_POP3,     "pops",         1,      995 },
#endif
#if LIBMUTTNG_SMTP
  { P_SMTP,     "smtp",         0,      25 },
  { P_SMTP,     "smtps",        1,      465 },
  { P_SMTP,     "ssmtp",        1,      465 },
#endif
  { P_LAST,     NULL,           0,      0 }
};

/**
 * For an URL string, check and return protocol.
 * @param s URL String.
 * @param secure Pointer to storage to indicate whether SSL is used or not.
 * @param defport Pointer to storage for default port
 * @param error Error buffer for error message.
 * @return
 *      - protocol upon success
 *      - @c P_LAST otherwise
 */
static urlproto_t url_get_proto (const char* s, bool* secure,
                                 unsigned short* defport,
                                 buffer_t* error) {
  char sbuf[32];        /* 32 should suffice for any ^[a-z]: string */
  char *t = NULL;
  int i = 0;

  if (!s || !(t = (char*) strchr (s, ':')) ||
      (t - s) + 1 >= (int) sizeof (sbuf)) {
    if (error)
      buffer_add_str (error, _("no or too long protocoll identifier in URL"), -1);
    return P_LAST;
  }

  strfcpy (sbuf, s, t - s + 1);
  for (t = sbuf; *t; t++)
    *t = tolower (*t);

  for (i = 0; Protos[i].str; i++)
    if (str_eq (Protos[i].str, sbuf)) {
      *defport = Protos[i].defport;
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
 * @param local Local character set.
 * @param error Error buffer.
 * @return Success.
 */
static bool parse_userhost (url_t* url, char* src, buffer_t* error, const char* local) {
  char *t;
  char *p;
  char *path;
  char *err;
  size_t chars = 0;
  size_t pathlen = 0;

  url->username = NULL;
  url->password = NULL;
  url->host = NULL;
  url->idn_host = NULL;
  url->port = url->defport;
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
      if (!url_decode (&url->password, &chars)) {
        err = url->password;
        goto decode_error;
      }
    }
    if (str_len (src) == 0)
      goto field_error;
    url->username = str_dup (src);
    if (!url_decode (&url->username, &chars)) {
      err = url->username;
      goto decode_error;
    }
  }
  else
    t = src;

  /* see if we have either ] for IPv6 addr or ':' */
  if ((p = strrchr(t,']')) || (p = strrchr (t, ':'))) {
    /* IPv6, only assume port if after IP addr we have ':' */
    if (*p == ':' || (*p == ']' && *(p+1) == ':' && p++)) {
      if (str_len (p+1) == 0)
        goto field_error;
      *p++ = '\0';
      url->port = atoi (p);
    }
  }

  if (t && *t) {
    size_t hostlen = str_len(t);
    /* skip [ and ] if present */
    url->host = str_substrdup(t+(*t=='['?1:0), t+hostlen-(t[hostlen-1]==']'?1:0));

    if (!url_decode (&url->host, &chars)) {
      err = url->host;
      goto decode_error;
    }

    buffer_t input,output;
    buffer_init(&input);
    buffer_init(&output);

    buffer_add_str(&input,url->host,-1);

    if (!net_local2idn(&output,&input,local)) {
      err = url->host;
      buffer_free(&input);
      buffer_free(&output);
      goto idn_error;
    }

    url->idn_host = str_dup(output.str);

    buffer_free(&input);
    buffer_free(&output);

  }

  /* always fully qualify path with leading '/' */
  pathlen = str_len (path);
  url->path = (char*) mem_malloc (pathlen+2);
  url->path[0] = '/';
  url->path[pathlen+1] = '\0';
  if (pathlen)
    memcpy (url->path+1, path, pathlen);
  if (!url_decode (&url->path, &chars)) {
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

idn_error:
  if (error)
    buffer_add_str (error, _("bad IDN in hostname"), -1);
  url_free (url);
  return (false);
}

void url_free (url_t* url) {
  mem_free (&url->username);
  mem_free (&url->password);
  mem_free (&url->host);
  mem_free (&url->idn_host);
  mem_free (&url->path);
}

url_t* url_from_string (const char* url, buffer_t* error, const char* local) {
  char* tmp = NULL, *buf = NULL;
  url_t* ret = NULL;
  urlproto_t proto = P_LAST;
  bool secure = false;
  unsigned short defport = 0;

  if ((proto = url_get_proto (url, &secure, &defport, error)) == P_LAST)
    return (NULL);

  ret = new url_t (proto, secure, defport);
  buf = str_dup (url);
  tmp = strchr (buf, ':') + 1;

  if (!parse_userhost (ret, tmp, error, local))
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
  unsigned short int i = 0;
  bool ip6 = false;

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
  ip6 = strchr(url->host,':')!=NULL;
  if (ip6) buffer_add_ch(dst,'[');
  buffer_add_str (dst, url->host, -1);
  if (ip6) buffer_add_ch(dst,']');
  if (url->port != url->defport) {
    buffer_add_ch (dst, ':');
    buffer_add_snum (dst, url->port, -1);
  }
  buffer_add_str (dst, url->path, -1);
}

bool url_eq (url_t* url1, url_t* url2, bool path) {
  if (!url1 || !url2)
    return (!url1 && !url2);
  return (str_eq (url1->username, url2->username) &&
          str_eq (url1->password, url2->password) &&
          str_eq (url1->host, url2->host) &&
          (!path || str_eq (url1->path, url2->path)) &&
          url1->port == url2->port &&
          url1->secure == url2->secure &&
          url1->proto == url2->proto);
}

unsigned short url_defport (urlproto_t proto, bool secure) {
  int i = 0;
  for (i = 0; Protos[i].str; i++)
    if (Protos[i].proto == proto && Protos[i].secure == secure)
      return Protos[i].defport;
  return 0;
}

void url_syntax(buffer_t* dst) {
  if (!dst) return;
  buffer_add_str(dst,"(file|",6);
#if LIBMUTTNG_POP3
  buffer_add_str(dst,"|pop[s]",7);
#endif
#if LIBMUTTNG_IMAP
  buffer_add_str(dst,"|imap[s]",8);
#endif
#if LIBMUTTNG_NNTP
  buffer_add_str(dst,"|nntp[s]",8);
#endif
#if LIBMUTTNG_SMTP
  buffer_add_str(dst,"|smtp[s]",8);
#endif
  buffer_add_str(dst,")://[user[@password]:]host[/path]",33);
}
