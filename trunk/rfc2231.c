/*
 * Copyright notice from original mutt:
 * Copyright (C) 1999-2000 Thomas Roessler <roessler@does-not-exist.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

/*
 * Yet another MIME encoding for header data.  This time, it's
 * parameters, specified in RFC 2231, and modeled after the
 * encoding used in URLs.
 * 
 * Additionally, continuations and encoding are mixed in an, errrm,
 * interesting manner.
 *
 */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "mutt.h"
#include "mime.h"
#include "charset.h"
#include "rfc2047.h"
#include "rfc2231.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

struct rfc2231_parameter {
  char *attribute;
  char *value;
  int index;
  int encoded;
  struct rfc2231_parameter
                   *next;
};

static char *rfc2231_get_charset (char *, char *, size_t);
static struct rfc2231_parameter *rfc2231_new_parameter (void);
static void rfc2231_decode_one (char *, char *);
static void rfc2231_free_parameter (struct rfc2231_parameter **);
static void rfc2231_join_continuations (PARAMETER **,
                                        struct rfc2231_parameter *);
static void rfc2231_list_insert (struct rfc2231_parameter **,
                                 struct rfc2231_parameter *);

static void purge_empty_parameters (PARAMETER ** headp)
{
  PARAMETER *p, *q, **last;

  for (last = headp, p = *headp; p; p = q) {
    q = p->next;
    if (!p->attribute || !p->value) {
      *last = q;
      p->next = NULL;
      mutt_free_parameter (&p);
    }
    else
      last = &p->next;
  }
}


void rfc2231_decode_parameters (PARAMETER ** headp)
{
  PARAMETER *head = NULL;
  PARAMETER **last;
  PARAMETER *p, *q;

  struct rfc2231_parameter *conthead = NULL;
  struct rfc2231_parameter *conttmp;

  char *s, *t;
  char charset[STRING];

  int encoded;
  int index;
  short dirty = 0;              /* set to 1 when we may have created
                                 * empty parameters.
                                 */

  if (!headp)
    return;

  purge_empty_parameters (headp);

  for (last = &head, p = *headp; p; p = q) {
    q = p->next;

    if (!(s = strchr (p->attribute, '*'))) {

      /* 
       * Using RFC 2047 encoding in MIME parameters is explicitly
       * forbidden by that document.  Nevertheless, it's being
       * generated by some software, including certain Lotus Notes to 
       * Internet Gateways.  So we actually decode it.
       */

      if (option (OPTRFC2047PARAMS) && p->value && strstr (p->value, "=?"))
        rfc2047_decode (&p->value);
      else if (!option (OPTSTRICTMIME)) {
        if (ascii_strcasecmp (AssumedCharset, "us-ascii"))
          mutt_convert_nonmime_string (&p->value);
      }

      *last = p;
      last = &p->next;
      p->next = NULL;
    }
    else if (*(s + 1) == '\0') {
      *s = '\0';

      s = rfc2231_get_charset (p->value, charset, sizeof (charset));
      rfc2231_decode_one (p->value, s);
      mutt_convert_string (&p->value, charset, Charset, M_ICONV_HOOK_FROM);

      *last = p;
      last = &p->next;
      p->next = NULL;

      dirty = 1;
    }
    else {
      *s = '\0';
      s++;                      /* let s point to the first character of index. */
      for (t = s; *t && isdigit ((unsigned char) *t); t++);
      encoded = (*t == '*');
      *t = '\0';

      index = atoi (s);

      conttmp = rfc2231_new_parameter ();
      conttmp->attribute = p->attribute;
      conttmp->value = p->value;
      conttmp->encoded = encoded;
      conttmp->index = index;

      p->attribute = NULL;
      p->value = NULL;
      FREE (&p);

      rfc2231_list_insert (&conthead, conttmp);
    }
  }

  if (conthead) {
    rfc2231_join_continuations (last, conthead);
    dirty = 1;
  }

  *headp = head;

  if (dirty)
    purge_empty_parameters (headp);
}

static struct rfc2231_parameter *rfc2231_new_parameter (void)
{
  return safe_calloc (sizeof (struct rfc2231_parameter), 1);
}

static void rfc2231_free_parameter (struct rfc2231_parameter **p)
{
  if (*p) {
    FREE (&(*p)->attribute);
    FREE (&(*p)->value);
    FREE (p);
  }
}

static char *rfc2231_get_charset (char *value, char *charset, size_t chslen)
{
  char *t, *u;

  if (!(t = strchr (value, '\''))) {
    charset[0] = '\0';
    return value;
  }

  *t = '\0';
  strfcpy (charset, value, chslen);

  if ((u = strchr (t + 1, '\'')))
    return u + 1;
  else
    return t + 1;
}

static void rfc2231_decode_one (char *dest, char *src)
{
  char *d;

  for (d = dest; *src; src++) {
    if (*src == '%' &&
        isxdigit ((unsigned char) *(src + 1)) &&
        isxdigit ((unsigned char) *(src + 2))) {
      *d++ = (hexval (*(src + 1)) << 4) | (hexval (*(src + 2)));
      src += 2;
    }
    else
      *d++ = *src;
  }

  *d = '\0';
}

/* insert parameter into an ordered list.
 * 
 * Primary sorting key: attribute
 * Secondary sorting key: index
 */

static void rfc2231_list_insert (struct rfc2231_parameter **list,
                                 struct rfc2231_parameter *par)
{
  struct rfc2231_parameter **last = list;
  struct rfc2231_parameter *p = *list, *q;
  int c;

  while (p) {
    last = &p->next;
    q = p;
    p = p->next;

    c = strcmp (par->value, q->value);
    if ((c > 0) || (c == 0 && par->index >= q->index))
      break;
  }

  par->next = p;
  *last = par;
}

/* process continuation parameters */

static void rfc2231_join_continuations (PARAMETER ** head,
                                        struct rfc2231_parameter *par)
{
  struct rfc2231_parameter *q;

  char attribute[STRING];
  char charset[STRING];
  char *value = NULL;
  char *valp;
  int encoded;

  size_t l, vl;

  while (par) {
    value = NULL;
    l = 0;

    strfcpy (attribute, par->attribute, sizeof (attribute));

    if ((encoded = par->encoded))
      valp = rfc2231_get_charset (par->value, charset, sizeof (charset));
    else
      valp = par->value;

    do {
      if (encoded && par->encoded)
        rfc2231_decode_one (par->value, valp);

      vl = mutt_strlen (par->value);

      safe_realloc (&value, l + vl + 1);
      strcpy (value + l, par->value);   /* __STRCPY_CHECKED__ */
      l += vl;

      q = par->next;
      rfc2231_free_parameter (&par);
      if ((par = q))
        valp = par->value;
    } while (par && !strcmp (par->attribute, attribute));

    if (value) {
      if (encoded)
        mutt_convert_string (&value, charset, Charset, M_ICONV_HOOK_FROM);
      *head = mutt_new_parameter ();
      (*head)->attribute = safe_strdup (attribute);
      (*head)->value = value;
      head = &(*head)->next;
    }
  }
}

int rfc2231_encode_string (char **pd)
{
  int ext = 0, encode = 0;
  char *charset, *s, *t, *e, *d = 0;
  size_t slen, dlen = 0;

  /* 
   * A shortcut to detect pure 7bit data.
   * 
   * This should prevent the worst when character set handling
   * is flawed.
   */

  for (s = *pd; *s; s++)
    if (*s & 0x80)
      break;

  if (!*s)
    return 0;

  if (!Charset || !SendCharset ||
      !(charset = mutt_choose_charset (Charset, SendCharset,
                                       *pd, mutt_strlen (*pd), &d, &dlen))) {
    charset = safe_strdup (Charset ? Charset : "unknown-8bit");
    d = *pd;
    dlen = mutt_strlen (d);
  }

  if (!mutt_is_us_ascii (charset))
    encode = 1;

  for (s = d, slen = dlen; slen; s++, slen--)
    if (*s < 0x20 || *s >= 0x7f)
      encode = 1, ++ext;
    else if (strchr (MimeSpecials, *s) || strchr ("*'%", *s))
      ++ext;

  if (encode) {
    e = safe_malloc (dlen + 2 * ext + mutt_strlen (charset) + 3);
    sprintf (e, "%s''", charset);       /* __SPRINTF_CHECKED__ */
    t = e + mutt_strlen (e);
    for (s = d, slen = dlen; slen; s++, slen--)
      if (*s < 0x20 || *s >= 0x7f ||
          strchr (MimeSpecials, *s) || strchr ("*'%", *s)) {
        sprintf (t, "%%%02X", (unsigned char) *s);
        t += 3;
      }
      else
        *t++ = *s;
    *t = '\0';

    if (d != *pd)
      FREE (&d);
    FREE (pd);
    *pd = e;
  }
  else if (d != *pd) {
    FREE (pd);
    *pd = d;
  }

  FREE (&charset);

  return encode;
}
