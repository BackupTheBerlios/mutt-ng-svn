/*
 * Copyright notice from original mutt:
 * Copyright (C) 2000 Thomas Roessler <roessler@does-not-exist.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

/*
 * A simple URL parser.
 */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "mutt.h"
#include "ascii.h"
#include "mapping.h"
#include "url.h"

#include "mime.h"

#include "lib/mem.h"

#include <ctype.h>

static struct mapping_t UrlMap[] = {
  {"file", U_FILE},
  {"imap", U_IMAP},
  {"imaps", U_IMAPS},
  {"pop", U_POP},
  {"pops", U_POPS},
  {"nntp", U_NNTP},
  {"news", U_NNTP},
  {"nntps", U_NNTPS},
  {"snews", U_NNTPS},
  {"mailto", U_MAILTO},
  {NULL, U_UNKNOWN}
};


static void url_pct_decode (char *s)
{
  char *d;

  if (!s)
    return;

  for (d = s; *s; s++) {
    if (*s == '%' && s[1] && s[2] &&
        isxdigit ((unsigned char) s[1]) &&
        isxdigit ((unsigned char) s[2]) &&
        hexval (s[1]) >= 0 && hexval (s[2]) >= 0) {
      *d++ = (hexval (s[1]) << 4) | (hexval (s[2]));
      s += 2;
    }
    else
      *d++ = *s;
  }
  *d = '\0';
}

url_scheme_t url_check_scheme (const char *s)
{
  char sbuf[STRING];
  char *t;
  int i;

  if (!s || !(t = strchr (s, ':')))
    return U_UNKNOWN;
  if ((t - s) + 1 >= sizeof (sbuf))
    return U_UNKNOWN;

  strfcpy (sbuf, s, t - s + 1);
  for (t = sbuf; *t; t++)
    *t = ascii_tolower (*t);

  if ((i = mutt_getvaluebyname (sbuf, UrlMap)) == -1)
    return U_UNKNOWN;
  else
    return (url_scheme_t) i;
}

int url_parse_file (char *d, const char *src, size_t dl)
{
  if (ascii_strncasecmp (src, "file:", 5))
    return -1;
  else if (!ascii_strncasecmp (src, "file://", 7))      /* we don't support remote files */
    return -1;
  else
    strfcpy (d, src + 5, dl);

  url_pct_decode (d);
  return 0;
}

/* ciss_parse_userhost: fill in components of ciss with info from src. Note
 *   these are pointers into src, which is altered with '\0's. Port of 0
 *   means no port given. */
static char *ciss_parse_userhost (ciss_url_t * ciss, char *src)
{
  char *t;
  char *p;
  char *path;

  ciss->user = NULL;
  ciss->pass = NULL;
  ciss->host = NULL;
  ciss->port = 0;

  if (strncmp (src, "//", 2))
    return src;

  src += 2;

  if ((path = strchr (src, '/')))
    *path++ = '\0';

  if ((t = strrchr (src, '@'))) {
    *t = '\0';
    if ((p = strchr (src, ':'))) {
      *p = '\0';
      ciss->pass = p + 1;
      url_pct_decode (ciss->pass);
    }
    ciss->user = src;
    url_pct_decode (ciss->user);
    t++;
  }
  else
    t = src;

  if ((p = strchr (t, ':'))) {
    *p++ = '\0';
    ciss->port = atoi (p);
  }
  else
    ciss->port = 0;

  ciss->host = t;
  url_pct_decode (ciss->host);
  return path;
}

/* url_parse_ciss: Fill in ciss_url_t. char* elements are pointers into src,
 *   which is modified by this call (duplicate it first if you need to). */
int url_parse_ciss (ciss_url_t * ciss, char *src)
{
  char *tmp;

  if ((ciss->scheme = url_check_scheme (src)) == U_UNKNOWN)
    return -1;

  tmp = strchr (src, ':') + 1;

  ciss->path = ciss_parse_userhost (ciss, tmp);
  url_pct_decode (ciss->path);

  return 0;
}

/* url_ciss_tostring: output the URL string for a given CISS object. */

int url_ciss_tostring (ciss_url_t * ciss, char *dest, size_t len, int flags)
{
  long l;

  if (ciss->scheme == U_UNKNOWN)
    return -1;

  snprintf (dest, len, "%s:", mutt_getnamebyvalue (ciss->scheme, UrlMap));

  if (ciss->host) {
    str_cat (dest, len, "//");
    len -= (l = str_len (dest));
    dest += l;

    if (ciss->user) {
      if (flags & U_DECODE_PASSWD && ciss->pass)
        snprintf (dest, len, "%s:%s@", ciss->user, ciss->pass);
      else
        snprintf (dest, len, "%s@", ciss->user);

      len -= (l = str_len (dest));
      dest += l;
    }

    if (ciss->port)
      snprintf (dest, len, "%s:%hu/", ciss->host, ciss->port);
    else
      snprintf (dest, len, "%s/", ciss->host);
  }

  if (ciss->path)
    str_cat (dest, len, ciss->path);

  return 0;
}

int url_parse_mailto (ENVELOPE * e, char **body, const char *src)
{
  char *t;
  char *tmp;
  char *headers;
  char *tag, *value;
  char scratch[HUGE_STRING];

  int taglen;

  LIST *last = NULL;

  if (!(t = strchr (src, ':')))
    return -1;

  if ((tmp = str_dup (t + 1)) == NULL)
    return -1;

  if ((headers = strchr (tmp, '?')))
    *headers++ = '\0';

  url_pct_decode (tmp);
  e->to = rfc822_parse_adrlist (e->to, tmp);

  tag = headers ? strtok (headers, "&") : NULL;

  for (; tag; tag = strtok (NULL, "&")) {
    if ((value = strchr (tag, '=')))
      *value++ = '\0';
    if (!value || !*value)
      continue;

    url_pct_decode (tag);
    url_pct_decode (value);

    if (!ascii_strcasecmp (tag, "body")) {
      if (body)
        str_replace (body, value);
    }
    else {
#define SAFEPFX (option (OPTSTRICTMAILTO) ? "" : "X-Mailto-")
      taglen = str_len (tag) + str_len (SAFEPFX);
      /* mutt_parse_rfc822_line makes some assumptions */
      snprintf (scratch, sizeof (scratch), "%s%s: %s", SAFEPFX, tag, value);
#undef SAVEPFX
      scratch[taglen] = '\0';
      value = &scratch[taglen + 1];
      SKIPWS (value);
      mutt_parse_rfc822_line (e, NULL, scratch, value, 1, 0, 0, &last);
      /* if $strict_mailto is set, force editing headers to let
       * users have a look at what we got */
      if (!option (OPTSTRICTMAILTO)) {
        set_option (OPTXMAILTO);
        set_option (OPTEDITHDRS);
      }
    }
  }

  mem_free (&tmp);
  return 0;
}
