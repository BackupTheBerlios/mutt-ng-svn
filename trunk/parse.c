/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "mutt.h"
#include "buffer.h"
#include "enter.h"
#include "ascii.h"
#include "recvattach.h"
#include "mx.h"
#include "mime.h"
#include "rfc2047.h"
#include "rfc2231.h"
#include "mutt_crypt.h"
#include "url.h"

#include "lib/mem.h"
#include "lib/intl.h"
#include "lib/str.h"
#include "lib/rx.h"
#include "lib/debug.h"

#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <stdlib.h>

/* Reads an arbitrarily long header field, and looks ahead for continuation
 * lines.  ``line'' must point to a dynamically allocated string; it is
 * increased if more space is required to fit the whole line.
 */
char *mutt_read_rfc822_line (FILE * f, char *line, size_t * linelen)
{
  char *buf = line;
  char ch;
  size_t offset = 0;

  FOREVER {
    if (fgets (buf, *linelen - offset, f) == NULL ||    /* end of file or */
        (ISSPACE (*line) && !offset)) { /* end of headers */
      *line = 0;
      return (line);
    }

    buf += str_len (buf) - 1;
    if (*buf == '\n') {
      /* we did get a full line. remove trailing space */
      while (ISSPACE (*buf))
        *buf-- = 0;             /* we cannot come beyond line's beginning because
                                 * it begins with a non-space */

      /* check to see if the next line is a continuation line */
      if ((ch = fgetc (f)) != ' ' && ch != '\t') {
        ungetc (ch, f);
        return (line);          /* next line is a separate header field or EOH */
      }

      /* eat tabs and spaces from the beginning of the continuation line */
      while ((ch = fgetc (f)) == ' ' || ch == '\t');
      ungetc (ch, f);
      *++buf = ' ';             /* string is still terminated because we removed
                                   at least one whitespace char above */
    }

    buf++;
    offset = buf - line;
    if (*linelen < offset + STRING) {
      /* grow the buffer */
      *linelen += STRING;
      mem_realloc (&line, *linelen);
      buf = line + offset;
    }
  }
  /* not reached */
}

LIST *mutt_parse_references (char *s, int in_reply_to)
{
  LIST *t, *lst = NULL;
  int m, n = 0;
  char *o = NULL, *new, *at;

  while ((s = strtok (s, " \t;")) != NULL) {
    /*
     * some mail clients add other garbage besides message-ids, so do a quick
     * check to make sure this looks like a valid message-id
     * some idiotic clients also break their message-ids between lines, deal
     * with that too (give up if it's more than two lines, though)
     */
    t = NULL;
    new = NULL;

    if (*s == '<') {
      n = str_len (s);
      if (s[n - 1] != '>') {
        o = s;
        s = NULL;
        continue;
      }

      new = str_dup (s);
    }
    else if (o) {
      m = str_len (s);
      if (s[m - 1] == '>') {
        new = mem_malloc (sizeof (char) * (n + m + 1));
        strcpy (new, o);        /* __STRCPY_CHECKED__ */
        strcpy (new + n, s);    /* __STRCPY_CHECKED__ */
      }
    }
    if (new) {
      /* make sure that this really does look like a message-id.
       * it should have exactly one @, and if we're looking at
       * an in-reply-to header, make sure that the part before
       * the @ has more than eight characters or it's probably
       * an email address
       */
      if (!(at = strchr (new, '@')) || strchr (at + 1, '@')
          || (in_reply_to && at - new <= 8))
        mem_free (&new);
      else {
        t = (LIST *) mem_malloc (sizeof (LIST));
        t->data = new;
        t->next = lst;
        lst = t;
      }
    }
    o = NULL;
    s = NULL;
  }

  return (lst);
}

int mutt_check_encoding (const char *c)
{
  if (ascii_strncasecmp ("7bit", c, sizeof ("7bit") - 1) == 0)
    return (ENC7BIT);
  else if (ascii_strncasecmp ("8bit", c, sizeof ("8bit") - 1) == 0)
    return (ENC8BIT);
  else if (ascii_strncasecmp ("binary", c, sizeof ("binary") - 1) == 0)
    return (ENCBINARY);
  else
    if (ascii_strncasecmp
        ("quoted-printable", c, sizeof ("quoted-printable") - 1) == 0)
    return (ENCQUOTEDPRINTABLE);
  else if (ascii_strncasecmp ("base64", c, sizeof ("base64") - 1) == 0)
    return (ENCBASE64);
  else if (ascii_strncasecmp ("x-uuencode", c, sizeof ("x-uuencode") - 1) ==
           0)
    return (ENCUUENCODED);
#ifdef SUN_ATTACHMENT
  else if (ascii_strncasecmp ("uuencode", c, sizeof ("uuencode") - 1) == 0)
    return (ENCUUENCODED);
#endif
  else
    return (ENCOTHER);
}

static PARAMETER *parse_parameters (const char *s)
{
  PARAMETER *head = 0, *cur = 0, *new;
  char buffer[LONG_STRING];
  const char *p;
  size_t i;

  debug_print (2, ("`%s'\n", s));

  while (*s) {
    if ((p = strpbrk (s, "=;")) == NULL) {
      debug_print (1, ("malformed parameter: %s\n", s));
      goto bail;
    }

    /* if we hit a ; now the parameter has no value, just skip it */
    if (*p != ';') {
      i = p - s;

      new = mutt_new_parameter ();

      new->attribute = mem_malloc (i + 1);
      memcpy (new->attribute, s, i);
      new->attribute[i] = 0;

      /* remove whitespace from the end of the attribute name */
      while (ISSPACE (new->attribute[--i]))
        new->attribute[i] = 0;

      s = p + 1;                /* skip over the = */
      SKIPWS (s);

      if (*s == '"') {
        int state_ascii = 1;

        s++;
        for (i = 0; *s && i < sizeof (buffer) - 1; i++, s++) {
          if (!option (OPTSTRICTMIME)) {
            /* As iso-2022-* has a characer of '"' with non-ascii state,
             * ignore it. */
            if (*s == 0x1b && i < sizeof (buffer) - 2) {
              if (s[1] == '(' && (s[2] == 'B' || s[2] == 'J'))
                state_ascii = 1;
              else
                state_ascii = 0;
            }
          }
          if (state_ascii && *s == '"')
            break;
          if (*s == '\\') {
            /* Quote the next character */
            buffer[i] = s[1];
            if (!*++s)
              break;
          }
          else
            buffer[i] = *s;
        }
        buffer[i] = 0;
        if (*s)
          s++;                  /* skip over the " */
      }
      else {
        for (i = 0; *s && *s != ' ' && *s != ';' && i < sizeof (buffer) - 1;
             i++, s++)
          buffer[i] = *s;
        buffer[i] = 0;
      }

      new->value = str_dup (buffer);

      debug_print (2, ("`%s' = `%s'\n", new->attribute ? new->attribute : "", 
                  new->value ? new->value : ""));

      /* Add this parameter to the list */
      if (head) {
        cur->next = new;
        cur = cur->next;
      }
      else
        head = cur = new;
    }
    else {
      debug_print (1, ("parameter with no value: %s\n", s));
      s = p;
    }

    /* Find the next parameter */
    if (*s != ';' && (s = strchr (s, ';')) == NULL)
      break;                    /* no more parameters */

    do {
      s++;

      /* Move past any leading whitespace */
      SKIPWS (s);
    }
    while (*s == ';');          /* skip empty parameters */
  }

bail:

  rfc2231_decode_parameters (&head);
  return (head);
}

int mutt_check_mime_type (const char *s)
{
  if (ascii_strcasecmp ("text", s) == 0)
    return TYPETEXT;
  else if (ascii_strcasecmp ("multipart", s) == 0)
    return TYPEMULTIPART;
#ifdef SUN_ATTACHMENT
  else if (ascii_strcasecmp ("x-sun-attachment", s) == 0)
    return TYPEMULTIPART;
#endif
  else if (ascii_strcasecmp ("application", s) == 0)
    return TYPEAPPLICATION;
  else if (ascii_strcasecmp ("message", s) == 0)
    return TYPEMESSAGE;
  else if (ascii_strcasecmp ("image", s) == 0)
    return TYPEIMAGE;
  else if (ascii_strcasecmp ("audio", s) == 0)
    return TYPEAUDIO;
  else if (ascii_strcasecmp ("video", s) == 0)
    return TYPEVIDEO;
  else if (ascii_strcasecmp ("model", s) == 0)
    return TYPEMODEL;
  else if (ascii_strcasecmp ("*", s) == 0)
    return TYPEANY;
  else if (ascii_strcasecmp (".*", s) == 0)
    return TYPEANY;
  else
    return TYPEOTHER;
}

void mutt_parse_content_type (char *s, BODY * ct)
{
  char *pc;
  char *subtype;

  mem_free (&ct->subtype);
  mutt_free_parameter (&ct->parameter);

  /* First extract any existing parameters */
  if ((pc = strchr (s, ';')) != NULL) {
    *pc++ = 0;
    while (*pc && ISSPACE (*pc))
      pc++;
    ct->parameter = parse_parameters (pc);

    /* Some pre-RFC1521 gateways still use the "name=filename" convention,
     * but if a filename has already been set in the content-disposition,
     * let that take precedence, and don't set it here */
    if ((pc = mutt_get_parameter ("name", ct->parameter)) != 0
        && !ct->filename)
      ct->filename = str_dup (pc);

#ifdef SUN_ATTACHMENT
    /* this is deep and utter perversion */
    if ((pc = mutt_get_parameter ("conversions", ct->parameter)) != 0)
      ct->encoding = mutt_check_encoding (pc);
#endif

  }

  /* Now get the subtype */
  if ((subtype = strchr (s, '/'))) {
    *subtype++ = '\0';
    for (pc = subtype; *pc && !ISSPACE (*pc) && *pc != ';'; pc++);
    *pc = '\0';
    ct->subtype = str_dup (subtype);
  }

  /* Finally, get the major type */
  ct->type = mutt_check_mime_type (s);

#ifdef SUN_ATTACHMENT
  if (ascii_strcasecmp ("x-sun-attachment", s) == 0)
    ct->subtype = str_dup ("x-sun-attachment");
#endif

  if (ct->type == TYPEOTHER) {
    ct->xtype = str_dup (s);
  }

  if (ct->subtype == NULL) {
    /* Some older non-MIME mailers (i.e., mailtool, elm) have a content-type
     * field, so we can attempt to convert the type to BODY here.
     */
    if (ct->type == TYPETEXT)
      ct->subtype = str_dup ("plain");
    else if (ct->type == TYPEAUDIO)
      ct->subtype = str_dup ("basic");
    else if (ct->type == TYPEMESSAGE)
      ct->subtype = str_dup ("rfc822");
    else if (ct->type == TYPEOTHER) {
      char buffer[SHORT_STRING];

      ct->type = TYPEAPPLICATION;
      snprintf (buffer, sizeof (buffer), "x-%s", s);
      ct->subtype = str_dup (buffer);
    }
    else
      ct->subtype = str_dup ("x-unknown");
  }

  /* Default character set for text types. */
  if (ct->type == TYPETEXT) {
    if (!(pc = mutt_get_parameter ("charset", ct->parameter)))
      mutt_set_parameter ("charset", option (OPTSTRICTMIME) ? "us-ascii" :
                          (const char *)
                          mutt_get_first_charset (AssumedCharset),
                          &ct->parameter);
  }

}

static void parse_content_disposition (char *s, BODY * ct)
{
  PARAMETER *parms;

  if (!ascii_strncasecmp ("inline", s, 6))
    ct->disposition = DISPINLINE;
  else if (!ascii_strncasecmp ("form-data", s, 9))
    ct->disposition = DISPFORMDATA;
  else
    ct->disposition = DISPATTACH;

  /* Check to see if a default filename was given */
  if ((s = strchr (s, ';')) != NULL) {
    s++;
    SKIPWS (s);
    if ((s =
         mutt_get_parameter ("filename",
                             (parms = parse_parameters (s)))) != 0)
      str_replace (&ct->filename, s);
    if ((s = mutt_get_parameter ("name", parms)) != 0)
      ct->form_name = str_dup (s);
    mutt_free_parameter (&parms);
  }
}

/* args:
 *	fp	stream to read from
 *
 *	digest	1 if reading subparts of a multipart/digest, 0
 *		otherwise
 */

BODY *mutt_read_mime_header (FILE * fp, int digest)
{
  BODY *p = mutt_new_body ();
  char *c;
  char *line = mem_malloc (LONG_STRING);
  size_t linelen = LONG_STRING;

  p->hdr_offset = ftello (fp);

  p->encoding = ENC7BIT;        /* default from RFC1521 */
  p->type = digest ? TYPEMESSAGE : TYPETEXT;
  p->disposition = DISPINLINE;

  while (*(line = mutt_read_rfc822_line (fp, line, &linelen)) != 0) {
    /* Find the value of the current header */
    if ((c = strchr (line, ':'))) {
      *c = 0;
      c++;
      SKIPWS (c);
      if (!*c) {
        debug_print (1, ("skipping empty header field: %s\n", line));
        continue;
      }
    }
    else {
      debug_print (1, ("bogus MIME header: %s\n", line));
      break;
    }

    if (!ascii_strncasecmp ("content-", line, 8)) {
      if (!ascii_strcasecmp ("type", line + 8))
        mutt_parse_content_type (c, p);
      else if (!ascii_strcasecmp ("transfer-encoding", line + 8))
        p->encoding = mutt_check_encoding (c);
      else if (!ascii_strcasecmp ("disposition", line + 8))
        parse_content_disposition (c, p);
      else if (!ascii_strcasecmp ("description", line + 8)) {
        str_replace (&p->description, c);
        rfc2047_decode (&p->description);
      }
    }
#ifdef SUN_ATTACHMENT
    else if (!ascii_strncasecmp ("x-sun-", line, 6)) {
      if (!ascii_strcasecmp ("data-type", line + 6))
        mutt_parse_content_type (c, p);
      else if (!ascii_strcasecmp ("encoding-info", line + 6))
        p->encoding = mutt_check_encoding (c);
      else if (!ascii_strcasecmp ("content-lines", line + 6))
        mutt_set_parameter ("content-lines", c, &(p->parameter));
      else if (!ascii_strcasecmp ("data-description", line + 6)) {
        str_replace (&p->description, c);
        rfc2047_decode (&p->description);
      }
    }
#endif
  }
  p->offset = ftello (fp);       /* Mark the start of the real data */
  if (p->type == TYPETEXT && !p->subtype)
    p->subtype = str_dup ("plain");
  else if (p->type == TYPEMESSAGE && !p->subtype)
    p->subtype = str_dup ("rfc822");

  mem_free (&line);

  return (p);
}

void mutt_parse_part (FILE * fp, BODY * b)
{
  char *bound = 0;

  switch (b->type) {
  case TYPEMULTIPART:
#ifdef SUN_ATTACHMENT
    if (!ascii_strcasecmp (b->subtype, "x-sun-attachment"))
      bound = "--------";
    else
#endif
      bound = mutt_get_parameter ("boundary", b->parameter);

    fseeko (fp, b->offset, SEEK_SET);
    b->parts = mutt_parse_multipart (fp, bound,
                                     b->offset + b->length,
                                     ascii_strcasecmp ("digest",
                                                       b->subtype) == 0);
    break;

  case TYPEMESSAGE:
    if (b->subtype) {
      fseeko (fp, b->offset, SEEK_SET);
      if (mutt_is_message_type (b->type, b->subtype))
        b->parts = mutt_parse_messageRFC822 (fp, b);
      else if (ascii_strcasecmp (b->subtype, "external-body") == 0)
        b->parts = mutt_read_mime_header (fp, 0);
      else
        return;
    }
    break;

  default:
    return;
  }

  /* try to recover from parsing error */
  if (!b->parts) {
    b->type = TYPETEXT;
    str_replace (&b->subtype, "plain");
  }
}

/* parse a MESSAGE/RFC822 body
 *
 * args:
 *	fp		stream to read from
 *
 *	parent		structure which contains info about the message/rfc822
 *			body part
 *
 * NOTE: this assumes that `parent->length' has been set!
 */

BODY *mutt_parse_messageRFC822 (FILE * fp, BODY * parent)
{
  BODY *msg;

  parent->hdr = mutt_new_header ();
  parent->hdr->offset = ftello (fp);
  parent->hdr->env = mutt_read_rfc822_header (fp, parent->hdr, 0, 0);
  msg = parent->hdr->content;

  /* ignore the length given in the content-length since it could be wrong
     and we already have the info to calculate the correct length */
  /* if (msg->length == -1) */
  msg->length = parent->length - (msg->offset - parent->offset);

  /* if body of this message is empty, we can end up with a negative length */
  if (msg->length < 0)
    msg->length = 0;

  mutt_parse_part (fp, msg);
  return (msg);
}

/* parse a multipart structure
 *
 * args:
 *	fp		stream to read from
 *
 *	boundary	body separator
 *
 *	end_off		length of the multipart body (used when the final
 *			boundary is missing to avoid reading too far)
 *
 *	digest		1 if reading a multipart/digest, 0 otherwise
 */

BODY *mutt_parse_multipart (FILE * fp, const char *boundary, LOFF_T end_off,
                            int digest)
{
#ifdef SUN_ATTACHMENT
  int lines;
#endif
  int blen, len, crlf = 0;
  char buffer[LONG_STRING];
  BODY *head = 0, *last = 0, *new = 0;
  int i;
  int final = 0;                /* did we see the ending boundary? */

  if (!boundary) {
    mutt_error _("multipart message has no boundary parameter!");

    return (NULL);
  }

  blen = str_len (boundary);
  while (ftello (fp) < end_off && fgets (buffer, LONG_STRING, fp) != NULL) {
    len = str_len (buffer);

    crlf = (len > 1 && buffer[len - 2] == '\r') ? 1 : 0;

    if (buffer[0] == '-' && buffer[1] == '-' &&
        str_ncmp (buffer + 2, boundary, blen) == 0) {
      if (last) {
        last->length = ftello (fp) - last->offset - len - 1 - crlf;
        if (last->parts && last->parts->length == 0)
          last->parts->length =
            ftello (fp) - last->parts->offset - len - 1 - crlf;
        /* if the body is empty, we can end up with a -1 length */
        if (last->length < 0)
          last->length = 0;
      }

      /* Remove any trailing whitespace, up to the length of the boundary */
      for (i = len - 1; ISSPACE (buffer[i]) && i >= blen + 2; i--)
        buffer[i] = 0;

      /* Check for the end boundary */
      if (str_cmp (buffer + blen + 2, "--") == 0) {
        final = 1;
        break;                  /* done parsing */
      }
      else if (buffer[2 + blen] == 0) {
        new = mutt_read_mime_header (fp, digest);

#ifdef SUN_ATTACHMENT
        if (mutt_get_parameter ("content-lines", new->parameter)) {
          for (lines =
               atoi (mutt_get_parameter ("content-lines", new->parameter));
               lines; lines--)
            if (ftello (fp) >= end_off
                || fgets (buffer, LONG_STRING, fp) == NULL)
              break;
        }
#endif

        /*
         * Consistency checking - catch
         * bad attachment end boundaries
         */

        if (new->offset > end_off) {
          mutt_free_body (&new);
          break;
        }
        if (head) {
          last->next = new;
          last = new;
        }
        else
          last = head = new;
      }
    }
  }

  /* in case of missing end boundary, set the length to something reasonable */
  if (last && last->length == 0 && !final)
    last->length = end_off - last->offset;

  /* parse recursive MIME parts */
  for (last = head; last; last = last->next)
    mutt_parse_part (fp, last);

  return (head);
}

static const char *uncomment_timezone (char *buf, size_t buflen,
                                       const char *tz)
{
  char *p;
  size_t len;

  if (*tz != '(')
    return tz;                  /* no need to do anything */
  tz++;
  SKIPWS (tz);
  if ((p = strpbrk (tz, " )")) == NULL)
    return tz;
  len = p - tz;
  if (len > buflen - 1)
    len = buflen - 1;
  memcpy (buf, tz, len);
  buf[len] = 0;
  return buf;
}

static struct tz_t {
  char tzname[5];
  unsigned char zhours;
  unsigned char zminutes;
  unsigned char zoccident;      /* west of UTC? */
} TimeZones[] = {
  {
  "aat", 1, 0, 1},              /* Atlantic Africa Time */
  {
  "adt", 4, 0, 0},              /* Arabia DST */
  {
  "ast", 3, 0, 0},              /* Arabia */
    /*{ "ast",   4,  0, 1 }, *//* Atlantic */
  {
  "bst", 1, 0, 0},              /* British DST */
  {
  "cat", 1, 0, 0},              /* Central Africa */
  {
  "cdt", 5, 0, 1}, {
  "cest", 2, 0, 0},             /* Central Europe DST */
  {
  "cet", 1, 0, 0},              /* Central Europe */
  {
  "cst", 6, 0, 1},
    /*{ "cst",   8,  0, 0 }, *//* China */
    /*{ "cst",   9, 30, 0 }, *//* Australian Central Standard Time */
  {
  "eat", 3, 0, 0},              /* East Africa */
  {
  "edt", 4, 0, 1}, {
  "eest", 3, 0, 0},             /* Eastern Europe DST */
  {
  "eet", 2, 0, 0},              /* Eastern Europe */
  {
  "egst", 0, 0, 0},             /* Eastern Greenland DST */
  {
  "egt", 1, 0, 1},              /* Eastern Greenland */
  {
  "est", 5, 0, 1}, {
  "gmt", 0, 0, 0}, {
  "gst", 4, 0, 0},              /* Presian Gulf */
  {
  "hkt", 8, 0, 0},              /* Hong Kong */
  {
  "ict", 7, 0, 0},              /* Indochina */
  {
  "idt", 3, 0, 0},              /* Israel DST */
  {
  "ist", 2, 0, 0},              /* Israel */
    /*{ "ist",   5, 30, 0 }, *//* India */
  {
  "jst", 9, 0, 0},              /* Japan */
  {
  "kst", 9, 0, 0},              /* Korea */
  {
  "mdt", 6, 0, 1}, {
  "met", 1, 0, 0},              /* this is now officially CET */
  {
  "msd", 4, 0, 0},              /* Moscow DST */
  {
  "msk", 3, 0, 0},              /* Moscow */
  {
  "mst", 7, 0, 1}, {
  "nzdt", 13, 0, 0},            /* New Zealand DST */
  {
  "nzst", 12, 0, 0},            /* New Zealand */
  {
  "pdt", 7, 0, 1}, {
  "pst", 8, 0, 1}, {
  "sat", 2, 0, 0},              /* South Africa */
  {
  "smt", 4, 0, 0},              /* Seychelles */
  {
  "sst", 11, 0, 1},             /* Samoa */
    /*{ "sst",   8,  0, 0 }, *//* Singapore */
  {
  "utc", 0, 0, 0}, {
  "wat", 0, 0, 0},              /* West Africa */
  {
  "west", 1, 0, 0},             /* Western Europe DST */
  {
  "wet", 0, 0, 0},              /* Western Europe */
  {
  "wgst", 2, 0, 1},             /* Western Greenland DST */
  {
  "wgt", 3, 0, 1},              /* Western Greenland */
  {
  "wst", 8, 0, 0},              /* Western Australia */
};

/* parses a date string in RFC822 format:
 *
 * Date: [ weekday , ] day-of-month month year hour:minute:second timezone
 *
 * This routine assumes that `h' has been initialized to 0.  the `timezone'
 * field is optional, defaulting to +0000 if missing.
 */
time_t mutt_parse_date (const char *s, HEADER * h)
{
  int count = 0;
  char *t;
  int hour, min, sec;
  struct tm tm;
  int i;
  int tz_offset = 0;
  int zhours = 0;
  int zminutes = 0;
  int zoccident = 0;
  const char *ptz;
  char tzstr[SHORT_STRING];
  char scratch[SHORT_STRING];

  /* Don't modify our argument. Fixed-size buffer is ok here since
   * the date format imposes a natural limit. 
   */

  strfcpy (scratch, s, sizeof (scratch));

  /* kill the day of the week, if it exists. */
  if ((t = strchr (scratch, ',')))
    t++;
  else
    t = scratch;
  SKIPWS (t);

  memset (&tm, 0, sizeof (tm));

  while ((t = strtok (t, " \t")) != NULL) {
    switch (count) {
    case 0:                    /* day of the month */
      if (!isdigit ((unsigned char) *t))
        return (-1);
      tm.tm_mday = atoi (t);
      if (tm.tm_mday > 31)
        return (-1);
      break;

    case 1:                    /* month of the year */
      if ((i = mutt_check_month (t)) < 0)
        return (-1);
      tm.tm_mon = i;
      break;

    case 2:                    /* year */
      tm.tm_year = atoi (t);
      if (tm.tm_year < 50)
        tm.tm_year += 100;
      else if (tm.tm_year >= 1900)
        tm.tm_year -= 1900;
      break;

    case 3:                    /* time of day */
      if (sscanf (t, "%d:%d:%d", &hour, &min, &sec) == 3);
      else if (sscanf (t, "%d:%d", &hour, &min) == 2)
        sec = 0;
      else {
        debug_print (1, ("could not process time format: %s\n", t));
        return (-1);
      }
      tm.tm_hour = hour;
      tm.tm_min = min;
      tm.tm_sec = sec;
      break;

    case 4:                    /* timezone */
      /* sometimes we see things like (MST) or (-0700) so attempt to
       * compensate by uncommenting the string if non-RFC822 compliant
       */
      ptz = uncomment_timezone (tzstr, sizeof (tzstr), t);

      if (*ptz == '+' || *ptz == '-') {
        if (ptz[1] && ptz[2] && ptz[3] && ptz[4]
            && isdigit ((unsigned char) ptz[1])
            && isdigit ((unsigned char) ptz[2])
            && isdigit ((unsigned char) ptz[3])
            && isdigit ((unsigned char) ptz[4])) {
          zhours = (ptz[1] - '0') * 10 + (ptz[2] - '0');
          zminutes = (ptz[3] - '0') * 10 + (ptz[4] - '0');

          if (ptz[0] == '-')
            zoccident = 1;
        }
      }
      else {
        struct tz_t *tz;

        tz = bsearch (ptz, TimeZones, sizeof TimeZones / sizeof (struct tz_t),
                      sizeof (struct tz_t),
                      (int (*)(const void *, const void *)) ascii_strcasecmp
                      /* This is safe to do: A pointer to a struct equals
                       * a pointer to its first element*/ );

        if (tz) {
          zhours = tz->zhours;
          zminutes = tz->zminutes;
          zoccident = tz->zoccident;
        }

        /* ad hoc support for the European MET (now officially CET) TZ */
        if (ascii_strcasecmp (t, "MET") == 0) {
          if ((t = strtok (NULL, " \t")) != NULL) {
            if (!ascii_strcasecmp (t, "DST"))
              zhours++;
          }
        }
      }
      tz_offset = zhours * 3600 + zminutes * 60;
      if (!zoccident)
        tz_offset = -tz_offset;
      break;
    }
    count++;
    t = 0;
  }

  if (count < 4) {              /* don't check for missing timezone */
    debug_print (1, ("error parsing date format, using received time\n"));
    return (-1);
  }

  if (h) {
    h->zhours = zhours;
    h->zminutes = zminutes;
    h->zoccident = zoccident;
  }

  return (mutt_mktime (&tm, 0) + tz_offset);
}

/* extract the first substring that looks like a message-id */
static char *extract_message_id (const char *s)
{
  const char *p;
  char *r;
  size_t l;

  if ((s = strchr (s, '<')) == NULL || (p = strchr (s, '>')) == NULL)
    return (NULL);
  l = (size_t) (p - s) + 1;
  r = mem_malloc (l + 1);
  memcpy (r, s, l);
  r[l] = 0;
  return (r);
}

void mutt_parse_mime_message (CONTEXT * ctx, HEADER * cur)
{
  MESSAGE *msg;
  int flags = 0;

  do {
    if (cur->content->type != TYPEMESSAGE
        && cur->content->type != TYPEMULTIPART)
      break;                     /* nothing to do */

    if (cur->content->parts)
      break;                     /* The message was parsed earlier. */

    if ((msg = mx_open_message (ctx, cur->msgno))) {
      mutt_parse_part (msg->fp, cur->content);

      if (WithCrypto)
        cur->security = crypt_query (cur->content);

      mx_close_message (&msg);
    }
  } while (0);
  mutt_count_body_parts (cur, flags | M_PARTS_RECOUNT);
}

int mutt_parse_rfc822_line (ENVELOPE * e, HEADER * hdr, char *line, char *p,
                            short user_hdrs, short weed, short do_2047,
                            LIST ** lastp)
{
  int matched = 0;
  LIST *last = NULL;

  if (lastp)
    last = *lastp;

  switch (ascii_tolower (line[0])) {
  case 'a':
    if (ascii_strcasecmp (line + 1, "pparently-to") == 0) {
      e->to = rfc822_parse_adrlist (e->to, p);
      matched = 1;
    }
    else if (ascii_strcasecmp (line + 1, "pparently-from") == 0) {
      e->from = rfc822_parse_adrlist (e->from, p);
      matched = 1;
    }
    break;

  case 'b':
    if (ascii_strcasecmp (line + 1, "cc") == 0) {
      e->bcc = rfc822_parse_adrlist (e->bcc, p);
      matched = 1;
    }
    break;

  case 'c':
    if (ascii_strcasecmp (line + 1, "c") == 0) {
      e->cc = rfc822_parse_adrlist (e->cc, p);
      matched = 1;
    }
    else if (ascii_strncasecmp (line + 1, "ontent-", 7) == 0) {
      if (ascii_strcasecmp (line + 8, "type") == 0) {
        if (hdr)
          mutt_parse_content_type (p, hdr->content);
        matched = 1;
      }
      else if (ascii_strcasecmp (line + 8, "transfer-encoding") == 0) {
        if (hdr)
          hdr->content->encoding = mutt_check_encoding (p);
        matched = 1;
      }
      else if (ascii_strcasecmp (line + 8, "length") == 0) {
        if (hdr) {
          if ((hdr->content->length = atoi (p)) < 0)
            hdr->content->length = -1;
        }
        matched = 1;
      }
      else if (ascii_strcasecmp (line + 8, "description") == 0) {
        if (hdr) {
          str_replace (&hdr->content->description, p);
          rfc2047_decode (&hdr->content->description);
        }
        matched = 1;
      }
      else if (ascii_strcasecmp (line + 8, "disposition") == 0) {
        if (hdr)
          parse_content_disposition (p, hdr->content);
        matched = 1;
      }
    }
    break;

  case 'd':
    if (!ascii_strcasecmp ("ate", line + 1)) {
      str_replace (&e->date, p);
      if (hdr)
        hdr->date_sent = mutt_parse_date (p, hdr);
      matched = 1;
    }
    break;

  case 'e':
    if (!ascii_strcasecmp ("xpires", line + 1) &&
        hdr && mutt_parse_date (p, NULL) < time (NULL))
      hdr->expired = 1;
    break;

  case 'f':
    if (!ascii_strcasecmp ("rom", line + 1)) {
      e->from = rfc822_parse_adrlist (e->from, p);
      /* don't leave from info NULL if there's an invalid address (or
       * whatever) in From: field; mutt would just display it as empty
       * and mark mail/(esp.) news article as your own. aaargh! this
       * bothered me for _years_ */
      if (!e->from) {
        e->from = rfc822_new_address ();
        e->from->personal = str_dup (p);
      }
      matched = 1;
    }
#ifdef USE_NNTP
    else if (!str_casecmp (line + 1, "ollowup-to")) {
      if (!e->followup_to) {
        str_skip_trailws (p);
        e->followup_to = str_dup (str_skip_initws (p));
      }
      matched = 1;
    }
#endif
    break;

  case 'i':
    if (!ascii_strcasecmp (line + 1, "n-reply-to")) {
      mutt_free_list (&e->in_reply_to);
      e->in_reply_to = mutt_parse_references (p, 1);
      matched = 1;
    }
    break;

  case 'l':
    if (!ascii_strcasecmp (line + 1, "ines")) {
      if (hdr) {
        hdr->lines = atoi (p);

        /* 
         * HACK - mutt has, for a very short time, produced negative
         * Lines header values.  Ignore them. 
         */
        if (hdr->lines < 0)
          hdr->lines = 0;
      }

      matched = 1;
    }
    else if (!ascii_strcasecmp (line + 1, "ist-Post")) {
      /* RFC 2369.  FIXME: We should ignore whitespace, but don't. */
      if (strncmp (p, "NO", 2)) {
        char *beg, *end;

        for (beg = strchr (p, '<'); beg; beg = strchr (end, ',')) {
          ++beg;
          if (!(end = strchr (beg, '>')))
            break;

          /* Take the first mailto URL */
          if (url_check_scheme (beg) == U_MAILTO) {
            mem_free (&e->list_post);
            e->list_post = str_substrdup (beg, end);
            break;
          }
        }
      }
      matched = 1;
    }
    break;

  case 'm':
    if (!ascii_strcasecmp (line + 1, "ime-version")) {
      if (hdr)
        hdr->mime = 1;
      matched = 1;
    }
    else if (!ascii_strcasecmp (line + 1, "essage-id")) {
      /* We add a new "Message-ID:" when building a message */
      mem_free (&e->message_id);
      e->message_id = extract_message_id (p);
      matched = 1;
    }
    else if (!ascii_strncasecmp (line + 1, "ail-", 4)) {
      if (!ascii_strcasecmp (line + 5, "reply-to")) {
        /* override the Reply-To: field */
        rfc822_free_address (&e->reply_to);
        e->reply_to = rfc822_parse_adrlist (e->reply_to, p);
        matched = 1;
      }
      else if (!ascii_strcasecmp (line + 5, "followup-to")) {
        e->mail_followup_to = rfc822_parse_adrlist (e->mail_followup_to, p);
        matched = 1;
      }
    }
    break;

#ifdef USE_NNTP
  case 'n':
    if (!str_casecmp (line + 1, "ewsgroups")) {
      mem_free (&e->newsgroups);
      str_skip_trailws (p);
      e->newsgroups = str_dup (str_skip_initws (p));
      matched = 1;
    }
    break;
#endif

  case 'o':
    /* field `Organization:' saves only for pager! */
    if (!str_casecmp (line + 1, "rganization")) {
      if (!e->organization && str_casecmp (p, "unknown"))
        e->organization = str_dup (p);
    }
    break;

  case 'r':
    if (!ascii_strcasecmp (line + 1, "eferences")) {
      mutt_free_list (&e->references);
      e->references = mutt_parse_references (p, 0);
      matched = 1;
    }
    else if (!ascii_strcasecmp (line + 1, "eply-to")) {
      e->reply_to = rfc822_parse_adrlist (e->reply_to, p);
      matched = 1;
    }
    else if (!ascii_strcasecmp (line + 1, "eturn-path")) {
      e->return_path = rfc822_parse_adrlist (e->return_path, p);
      matched = 1;
    }
    else if (!ascii_strcasecmp (line + 1, "eceived")) {
      if (hdr && !hdr->received) {
        char *d = strchr (p, ';');

        if (d)
          hdr->received = mutt_parse_date (d + 1, NULL);
      }
    }
    break;

  case 's':
    if (!ascii_strcasecmp (line + 1, "ubject")) {
      if (!e->subject)
        e->subject = str_dup (p);
      matched = 1;
    }
    else if (!ascii_strcasecmp (line + 1, "ender")) {
      e->sender = rfc822_parse_adrlist (e->sender, p);
      matched = 1;
    }
    else if (!ascii_strcasecmp (line + 1, "tatus")) {
      if (hdr) {
        while (*p) {
          switch (*p) {
          case 'r':
            hdr->replied = 1;
            break;
          case 'O':
            hdr->old = 1;
            break;
          case 'R':
            hdr->read = 1;
            break;
          }
          p++;
        }
      }
      matched = 1;
    }
    else if ((!ascii_strcasecmp ("upersedes", line + 1) ||
              !ascii_strcasecmp ("upercedes", line + 1)) && hdr)
      e->supersedes = str_dup (p);
    break;

  case 't':
    if (ascii_strcasecmp (line + 1, "o") == 0) {
      e->to = rfc822_parse_adrlist (e->to, p);
      matched = 1;
    }
    break;

  case 'x':
    if (ascii_strcasecmp (line + 1, "-status") == 0) {
      if (hdr) {
        while (*p) {
          switch (*p) {
          case 'A':
            hdr->replied = 1;
            break;
          case 'D':
            hdr->deleted = 1;
            break;
          case 'F':
            hdr->flagged = 1;
            break;
          default:
            break;
          }
          p++;
        }
      }
      matched = 1;
    }
    else if (ascii_strcasecmp (line + 1, "-label") == 0) {
      e->x_label = str_dup (p);
      matched = 1;
    }
#ifdef USE_NNTP
    else if (!str_casecmp (line + 1, "-comment-to")) {
      if (!e->x_comment_to)
        e->x_comment_to = str_dup (p);
      matched = 1;
    }
    else if (!str_casecmp (line + 1, "ref")) {
      if (!e->xref)
        e->xref = str_dup (p);
      matched = 1;
    }
#endif

  default:
    break;
  }

  /* Keep track of the user-defined headers */
  if (!matched && user_hdrs) {
    /* restore the original line */
    line[str_len (line)] = ':';

    if (weed && option (OPTWEED) && mutt_matches_ignore (line, Ignore)
        && !mutt_matches_ignore (line, UnIgnore))
      goto done;

    if (last) {
      last->next = mutt_new_list ();
      last = last->next;
    }
    else
      last = e->userhdrs = mutt_new_list ();
    last->data = str_dup (line);
    if (do_2047)
      rfc2047_decode (&last->data);
  }

done:

  *lastp = last;
  return matched;
}


/* mutt_read_rfc822_header() -- parses a RFC822 header
 *
 * Args:
 *
 * f		stream to read from
 *
 * hdr		header structure of current message (optional).
 * 
 * user_hdrs	If set, store user headers.  Used for recall-message and
 * 		postpone modes.
 * 
 * weed		If this parameter is set and the user has activated the
 * 		$weed option, honor the header weed list for user headers.
 * 	        Used for recall-message.
 * 
 * Returns:     newly allocated envelope structure.  You should free it by
 *              mutt_free_envelope() when envelope stay unneeded.
 */
ENVELOPE *mutt_read_rfc822_header (FILE * f, HEADER * hdr, short user_hdrs,
                                   short weed)
{
  ENVELOPE *e = mutt_new_envelope ();
  LIST *last = NULL;
  char *line = mem_malloc (LONG_STRING);
  char *p;
  LOFF_T loc;
  int matched;
  size_t linelen = LONG_STRING;
  char buf[LONG_STRING + 1];

  if (hdr) {
    if (hdr->content == NULL) {
      hdr->content = mutt_new_body ();

      /* set the defaults from RFC1521 */
      hdr->content->type = TYPETEXT;
      hdr->content->subtype = str_dup ("plain");
      hdr->content->encoding = ENC7BIT;
      hdr->content->length = -1;

      /* RFC 2183 says this is arbitrary */
      hdr->content->disposition = DISPINLINE;
    }
  }

  while ((loc = ftello (f)),
         *(line = mutt_read_rfc822_line (f, line, &linelen)) != 0) {
    matched = 0;

    if ((p = strpbrk (line, ": \t")) == NULL || *p != ':') {
      char return_path[LONG_STRING];
      time_t t;

      /* some bogus MTAs will quote the original "From " line */
      if (str_ncmp (">From ", line, 6) == 0)
        continue;               /* just ignore */
      else if (is_from (line, return_path, sizeof (return_path), &t)) {
        /* MH somtimes has the From_ line in the middle of the header! */
        if (hdr && !hdr->received)
          hdr->received = t - mutt_local_tz (t);
        continue;
      }

      fseeko (f, loc, 0);
      break;                    /* end of header */
    }

    *buf = '\0';

    if (mutt_match_spam_list (line, SpamList, buf, sizeof (buf))) {
      if (!rx_list_match (NoSpamList, line)) {

        /* if spam tag already exists, figure out how to amend it */
        if (e->spam && *buf) {
          /* If SpamSep defined, append with separator */
          if (SpamSep) {
            mutt_buffer_addstr (e->spam, SpamSep);
            mutt_buffer_addstr (e->spam, buf);
          }

          /* else overwrite */
          else {
            e->spam->dptr = e->spam->data;
            *e->spam->dptr = '\0';
            mutt_buffer_addstr (e->spam, buf);
          }
        }

        /* spam tag is new, and match expr is non-empty; copy */
        else if (!e->spam && *buf) {
          e->spam = mutt_buffer_from (NULL, buf);
        }

        /* match expr is empty; plug in null string if no existing tag */
        else if (!e->spam) {
          e->spam = mutt_buffer_from (NULL, "");
        }

        if (e->spam && e->spam->data)
          debug_print (5, ("spam = %s\n", e->spam->data));
      }
    }

    *p = 0;
    p++;
    SKIPWS (p);
    if (!*p)
      continue;                 /* skip empty header fields */

    matched =
      mutt_parse_rfc822_line (e, hdr, line, p, user_hdrs, weed, 1, &last);

  }

  mem_free (&line);

  if (hdr) {
    hdr->content->hdr_offset = hdr->offset;
    hdr->content->offset = ftello (f);
    rfc2047_decode_envelope (e);
    /* check for missing or invalid date */
    if (hdr->date_sent <= 0) {
      debug_print (1, ("no date found, using received "
                       "time from msg separator\n"));
      hdr->date_sent = hdr->received;
    }
  }

  return (e);
}

ADDRESS *mutt_parse_adrlist (ADDRESS * p, const char *s)
{
  const char *q;

  /* check for a simple whitespace separated list of addresses */
  if ((q = strpbrk (s, "\"<>():;,\\")) == NULL) {
    char tmp[HUGE_STRING];
    char *r;

    strfcpy (tmp, s, sizeof (tmp));
    r = tmp;
    while ((r = strtok (r, " \t")) != NULL) {
      p = rfc822_parse_adrlist (p, r);
      r = NULL;
    }
  }
  else
    p = rfc822_parse_adrlist (p, s);

  return p;
}


/* Compares mime types to the ok and except lists */
int count_body_parts_check(LIST **checklist, BODY *b, int dflt) {
  LIST *type;
  ATTACH_MATCH *a;
  
  /* If list is null, use default behavior. */
  if (! *checklist) {
    /*return dflt;*/
    return 0;
  }
  
  for (type = *checklist; type; type = type->next) {
    a = (ATTACH_MATCH *)type->data;
    debug_print(5, ("cbpc: %s %d/%s ?? %s/%s [%d]... ",
               dflt ? "[OK] " : "[EXCL] ",
               b->type, b->subtype, a->major, a->minor, a->major_int));
    if ((a->major_int == TYPEANY || a->major_int == b->type) &&
        !regexec(&a->minor_rx, b->subtype, 0, NULL, 0)) {
      debug_print(5, ("yes\n"));
      return 1;
    } else {
      debug_print(5, ("no\n"));
    }
  }
  return 0;
}

#define AT_COUNT(why) { shallcount = 1; }
#define AT_NOCOUNT(why) { shallcount = 0; }

int count_body_parts (BODY *body, int flags) { 
  int count = 0;
  int shallcount, shallrecurse;
  BODY *bp;
  
  if (body == NULL)
    return 0;
  
  for (bp = body; bp != NULL; bp = bp->next) {
    /* Initial disposition is to count and not to recurse this part. */
    AT_COUNT("default");
    shallrecurse = 0;
    
    debug_print(5, ("bp: desc=\"%s\"; fn=\"%s\", type=\"%d/%s\"\n",
               bp->description ? bp->description : ("none"),
               bp->filename ? bp->filename :
               bp->d_filename ? bp->d_filename : "(none)",
               bp->type, bp->subtype ? bp->subtype : "*"));
    
    if (bp->type == TYPEMESSAGE) {
      shallrecurse = 1;
      
      /* If it's an external body pointer, don't recurse it. */
      if (!ascii_strcasecmp (bp->subtype, "external-body"))
        shallrecurse = 0;
      
      /* Don't count containers if they're top-level. */
      if (flags & M_PARTS_TOPLEVEL)
        AT_NOCOUNT("top-level message/*");
    } else if (bp->type == TYPEMULTIPART) {
      /* Always recurse multiparts, except multipart/alternative. */
      shallrecurse = 1;
      if (!str_casecmp(bp->subtype, "alternative"))
        shallrecurse = 0;
      
      /* Don't count containers if they're top-level. */
      if (flags & M_PARTS_TOPLEVEL)
        AT_NOCOUNT("top-level multipart");
    }
    
    if (bp->disposition == DISPINLINE &&
        bp->type != TYPEMULTIPART && bp->type != TYPEMESSAGE && bp == body)
      AT_NOCOUNT("ignore fundamental inlines");
    
    /* If this body isn't scheduled for enumeration already, don't bother
     * profiling it further. */
    
    if (shallcount) {
      /* Turn off shallcount if message type is not in ok list,
       * or if it is in except list. Check is done separately for
       * inlines vs. attachments.
       */

      if (bp->disposition == DISPATTACH) {
        if (!count_body_parts_check(&AttachAllow, bp, 1))
          AT_NOCOUNT("attach not allowed");
        if (count_body_parts_check(&AttachExclude, bp, 0))
          AT_NOCOUNT("attach excluded");
      } else {
        if (!count_body_parts_check(&InlineAllow, bp, 1))
          AT_NOCOUNT("inline not allowed");
        if (count_body_parts_check(&InlineExclude, bp, 0))
          AT_NOCOUNT("excluded");
      }
    }
    
    if (shallcount)
      count++;
    bp->attach_qualifies = shallcount ? 1 : 0;
    
    debug_print(5, ("cbp: %08x shallcount = %d\n", (unsigned int)bp, shallcount));
    
    if (shallrecurse) {
      debug_print(5, ("cbp: %08x pre count = %d\n", (unsigned int)bp, count));
      bp->attach_count = count_body_parts(bp->parts, flags & ~M_PARTS_TOPLEVEL);
      count += bp->attach_count;
      debug_print(5, ("cbp: %08x post count = %d\n", (unsigned int)bp, count));
    }
  }
  
  debug_print(5, ("bp: return %d\n", count < 0 ? 0 : count));
  return count < 0 ? 0 : count;
}

int mutt_count_body_parts (HEADER *hdr, int flags) {
  if (!option (OPTCOUNTATTACH))
    return (0);
  if (hdr->attach_valid && !(flags & M_PARTS_RECOUNT))
    return hdr->attach_total;
  
  if (AttachAllow || AttachExclude || InlineAllow || InlineExclude)
    hdr->attach_total = count_body_parts(hdr->content, flags | M_PARTS_TOPLEVEL);
  else
    hdr->attach_total = 0;
  
  hdr->attach_valid = 1;
  return hdr->attach_total;
}
