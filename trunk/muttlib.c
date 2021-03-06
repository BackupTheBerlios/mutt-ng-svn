/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 * Copyright (C) 1999-2000 Thomas Roessler <roessler@does-not-exist.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "mutt.h"
#include "ascii.h"
#include "buffer.h"
#include "enter.h"
#include "mutt_curses.h"
#include "mime.h"
#include "mx.h"
#include "url.h"
#include "attach.h"

#include "reldate.h"
#include "version.h"

#ifdef USE_IMAP
#include "imap.h"
#include "imap/mx_imap.h"
#endif

#include "mutt_crypt.h"

#include "lib/mem.h"
#include "lib/intl.h"
#include "lib/str.h"
#include "lib/debug.h"

#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <utime.h>

#define SW              (option(OPTMBOXPANE)?SidebarWidth:0)

BODY *mutt_new_body (void)
{
  BODY *p = (BODY *) mem_calloc (1, sizeof (BODY));

  p->disposition = DISPATTACH;
  p->use_disp = 1;
  return (p);
}


/* Modified by blong to accept a "suggestion" for file name.  If
 * that file exists, then construct one with unique name but 
 * keep any extension.  This might fail, I guess.
 * Renamed to mutt_adv_mktemp so I only have to change where it's
 * called, and not all possible cases.
 */
void mutt_adv_mktemp (const char* dir, char *s, size_t l)
{
  char buf[_POSIX_PATH_MAX];
  char tmp[_POSIX_PATH_MAX];
  char *period;
  size_t sl;
  struct stat sb;

  strfcpy (buf, dir && *dir ? dir : NONULL (Tempdir), sizeof (buf));
  mutt_expand_path (buf, sizeof (buf));
  if (s[0] == '\0') {
    snprintf (s, l, "%s/muttXXXXXX", buf);
    mktemp (s);
  }
  else {
    strfcpy (tmp, s, sizeof (tmp));
    mutt_sanitize_filename (tmp, 1);
    snprintf (s, l, "%s/%s", buf, tmp);
    if (lstat (s, &sb) == -1 && errno == ENOENT)
      return;
    if ((period = strrchr (tmp, '.')) != NULL)
      *period = 0;
    snprintf (s, l, "%s/%s.XXXXXX", buf, tmp);
    mktemp (s);
    if (period != NULL) {
      *period = '.';
      sl = str_len (s);
      strfcpy (s + sl, period, l - sl);
    }
  }
}

/* create a send-mode duplicate from a receive-mode body */

int mutt_copy_body (FILE * fp, BODY ** tgt, BODY * src)
{
  char tmp[_POSIX_PATH_MAX];
  BODY *b;

  PARAMETER *par, **ppar;

  short use_disp;

  if (src->filename) {
    use_disp = 1;
    strfcpy (tmp, src->filename, sizeof (tmp));
  }
  else {
    use_disp = 0;
    tmp[0] = '\0';
  }

  mutt_adv_mktemp (NULL, tmp, sizeof (tmp));
  if (mutt_save_attachment (fp, src, tmp, 0, NULL) == -1)
    return -1;

  *tgt = mutt_new_body ();
  b = *tgt;

  memcpy (b, src, sizeof (BODY));
  b->parts = NULL;
  b->next = NULL;

  b->filename = str_dup (tmp);
  b->use_disp = use_disp;
  b->unlink = 1;

  if (mutt_is_text_part (b))
    b->noconv = 1;

  b->xtype = str_dup (b->xtype);
  b->subtype = str_dup (b->subtype);
  b->form_name = str_dup (b->form_name);
  b->filename = str_dup (b->filename);
  b->d_filename = str_dup (b->d_filename);
  b->description = str_dup (b->description);

  /* 
   * we don't seem to need the HEADER structure currently.
   * XXX - this may change in the future
   */

  if (b->hdr)
    b->hdr = NULL;

  /* copy parameters */
  for (par = b->parameter, ppar = &b->parameter; par;
       ppar = &(*ppar)->next, par = par->next) {
    *ppar = mutt_new_parameter ();
    (*ppar)->attribute = str_dup (par->attribute);
    (*ppar)->value = str_dup (par->value);
  }

  mutt_stamp_attachment (b);

  return 0;
}



void mutt_free_body (BODY ** p)
{
  BODY *a = *p, *b;

  while (a) {
    b = a;
    a = a->next;

    if (b->parameter)
      mutt_free_parameter (&b->parameter);
    if (b->unlink && b->filename) {
      debug_print (1, ("unlinking %s.\n", b->filename));
      unlink (b->filename);
    }
    else if (b->filename)
      debug_print (1, ("not unlinking %s.\n", b->filename));

    mem_free (&b->filename);
    mem_free (&b->content);
    mem_free (&b->xtype);
    mem_free (&b->subtype);
    mem_free (&b->description);
    mem_free (&b->form_name);

    if (b->hdr) {
      /* Don't free twice (b->hdr->content = b->parts) */
      b->hdr->content = NULL;
      mutt_free_header (&b->hdr);
    }

    if (b->parts)
      mutt_free_body (&b->parts);

    mem_free (&b);
  }

  *p = 0;
}

void mutt_free_parameter (PARAMETER ** p)
{
  PARAMETER *t = *p;
  PARAMETER *o;

  while (t) {
    mem_free (&t->attribute);
    mem_free (&t->value);
    o = t;
    t = t->next;
    mem_free (&o);
  }
  *p = 0;
}

HEADER *mutt_dup_header (HEADER * h)
{
  HEADER *hnew;

  hnew = mutt_new_header ();
  memcpy (hnew, h, sizeof (HEADER));
  return hnew;
}

void mutt_free_header (HEADER ** h)
{
  if (!h || !*h)
    return;
  mutt_free_envelope (&(*h)->env);
  mutt_free_body (&(*h)->content);
  mem_free (&(*h)->maildir_flags);
  mem_free (&(*h)->tree);
  mem_free (&(*h)->path);
#ifdef MIXMASTER
  mutt_free_list (&(*h)->chain);
#endif
#if defined USE_POP || defined USE_IMAP || defined USE_NNTP
  mem_free (&(*h)->data);
#endif
  mem_free (h);
}

/* returns true if the header contained in "s" is in list "t" */
int mutt_matches_ignore (const char *s, LIST * t)
{
  for (; t; t = t->next) {
    if (!ascii_strncasecmp (s, t->data, str_len (t->data))
        || *t->data == '*')
      return 1;
  }
  return 0;
}

/* prepend the path part of *path to *link */
void mutt_expand_link (char *newpath, const char *path, const char *link)
{
  const char *lb = NULL;
  size_t len;

  /* link is full path */
  if (*link == '/') {
    strfcpy (newpath, link, _POSIX_PATH_MAX);
    return;
  }

  if ((lb = strrchr (path, '/')) == NULL) {
    /* no path in link */
    strfcpy (newpath, link, _POSIX_PATH_MAX);
    return;
  }

  len = lb - path + 1;
  memcpy (newpath, path, len);
  strfcpy (newpath + len, link, _POSIX_PATH_MAX - len);
}

char *mutt_expand_path (char *s, size_t slen)
{
  return _mutt_expand_path (s, slen, 0);
}

char *_mutt_expand_path (char *s, size_t slen, int rx)
{
  char p[_POSIX_PATH_MAX] = "";
  char q[_POSIX_PATH_MAX] = "";
  char tmp[_POSIX_PATH_MAX];
  char *t;

  char *tail = "";

  int recurse = 0;

  do {
    recurse = 0;

    switch (*s) {
    case '~':
      {
        if (*(s + 1) == '/' || *(s + 1) == 0) {
          strfcpy (p, NONULL (Homedir), sizeof (p));
          tail = s + 1;
        }
        else {
          struct passwd *pw;

          if ((t = strchr (s + 1, '/')))
            *t = 0;

          if ((pw = getpwnam (s + 1))) {
            strfcpy (p, pw->pw_dir, sizeof (p));
            if (t) {
              *t = '/';
              tail = t;
            }
            else
              tail = "";
          }
          else {
            /* user not found! */
            if (t)
              *t = '/';
            *p = '\0';
            tail = s;
          }
        }
      }
      break;

    case '=':
    case '+':
      {
#ifdef USE_IMAP
        /* if folder = imap[s]://host/: don't append slash */
        if (imap_is_magic (NONULL (Maildir), NULL) == M_IMAP && 
            Maildir[str_len (Maildir) - 1] == '/')
          strfcpy (p, NONULL (Maildir), sizeof (p));
        else
#endif
          snprintf (p, sizeof (p), "%s/", NONULL (Maildir));

        tail = s + 1;
      }
      break;

      /* elm compatibility, @ expands alias to user name */

    case '@':
      {
        HEADER *h;
        ADDRESS *alias;

        if ((alias = mutt_lookup_alias (s + 1))) {
          h = mutt_new_header ();
          h->env = mutt_new_envelope ();
          h->env->from = h->env->to = alias;
          mutt_default_save (p, sizeof (p), h);
          h->env->from = h->env->to = NULL;
          mutt_free_header (&h);
          /* Avoid infinite recursion if the resulting folder starts with '@' */
          if (*p != '@')
            recurse = 1;

          tail = "";
        }
      }
      break;

    case '>':
      {
        strfcpy (p, NONULL (Inbox), sizeof (p));
        tail = s + 1;
      }
      break;

    case '<':
      {
        strfcpy (p, NONULL (Outbox), sizeof (p));
        tail = s + 1;
      }
      break;

    case '!':
      {
        if (*(s + 1) == '!') {
          strfcpy (p, NONULL (LastFolder), sizeof (p));
          tail = s + 2;
        }
        else {
          strfcpy (p, NONULL (Spoolfile), sizeof (p));
          tail = s + 1;
        }
      }
      break;

    case '-':
      {
        strfcpy (p, NONULL (LastFolder), sizeof (p));
        tail = s + 1;
      }
      break;

    case '^':
      {
        strfcpy (p, NONULL (CurrentFolder), sizeof (p));
        tail = s + 1;
      }
      break;

    default:
      {
        *p = '\0';
        tail = s;
      }
    }

    if (rx && *p && !recurse) {
      mutt_rx_sanitize_string (q, sizeof (q), p);
      snprintf (tmp, sizeof (tmp), "%s%s", q, tail);
    }
    else
      snprintf (tmp, sizeof (tmp), "%s%s", p, tail);

    strfcpy (s, tmp, slen);
  }
  while (recurse);

  return (s);
}

/* Extract the real name from /etc/passwd's GECOS field.
 * When set, honor the regular expression in GecosMask,
 * otherwise assume that the GECOS field is a 
 * comma-separated list.
 * Replace "&" by a capitalized version of the user's login
 * name.
 */

char *mutt_gecos_name (char *dest, size_t destlen, struct passwd *pw)
{
  regmatch_t pat_match[1];
  size_t pwnl;
  int idx;
  char *p;

  if (!pw || !pw->pw_gecos)
    return NULL;

  memset (dest, 0, destlen);

  if (GecosMask.rx) {
    if (regexec (GecosMask.rx, pw->pw_gecos, 1, pat_match, 0) == 0)
      strfcpy (dest, pw->pw_gecos + pat_match[0].rm_so,
               MIN (pat_match[0].rm_eo - pat_match[0].rm_so + 1, destlen));
  }
  else if ((p = strchr (pw->pw_gecos, ',')))
    strfcpy (dest, pw->pw_gecos, MIN (destlen, p - pw->pw_gecos + 1));
  else
    strfcpy (dest, pw->pw_gecos, destlen);

  pwnl = str_len (pw->pw_name);

  for (idx = 0; dest[idx]; idx++) {
    if (dest[idx] == '&') {
      memmove (&dest[idx + pwnl], &dest[idx + 1],
               MAX (destlen - idx - pwnl - 1, 0));
      memcpy (&dest[idx], pw->pw_name, MIN (destlen - idx - 1, pwnl));
      dest[idx] = toupper ((unsigned char) dest[idx]);
    }
  }

  return dest;
}


char *mutt_get_parameter (const char *s, PARAMETER * p)
{
  for (; p; p = p->next)
    if (ascii_strcasecmp (s, p->attribute) == 0)
      return (p->value);

  return NULL;
}

void mutt_set_parameter (const char *attribute, const char *value,
                         PARAMETER ** p)
{
  PARAMETER *q;

  if (!value) {
    mutt_delete_parameter (attribute, p);
    return;
  }

  for (q = *p; q; q = q->next) {
    if (ascii_strcasecmp (attribute, q->attribute) == 0) {
      str_replace (&q->value, value);
      return;
    }
  }

  q = mutt_new_parameter ();
  q->attribute = str_dup (attribute);
  q->value = str_dup (value);
  q->next = *p;
  *p = q;
}

void mutt_delete_parameter (const char *attribute, PARAMETER ** p)
{
  PARAMETER *q;

  for (q = *p; q; p = &q->next, q = q->next) {
    if (ascii_strcasecmp (attribute, q->attribute) == 0) {
      *p = q->next;
      q->next = NULL;
      mutt_free_parameter (&q);
      return;
    }
  }
}

/* returns 1 if Mutt can't display this type of data, 0 otherwise */
int mutt_needs_mailcap (BODY * m)
{
  switch (m->type) {
  case TYPETEXT:

    if (!ascii_strcasecmp ("plain", m->subtype) ||
        !ascii_strcasecmp ("rfc822-headers", m->subtype) ||
        !ascii_strcasecmp ("enriched", m->subtype))
      return 0;
    break;

  case TYPEAPPLICATION:
    if ((WithCrypto & APPLICATION_PGP) && mutt_is_application_pgp (m))
      return 0;
    if ((WithCrypto & APPLICATION_SMIME) && mutt_is_application_smime (m))
      return 0;
    break;

  case TYPEMULTIPART:
  case TYPEMESSAGE:
    return 0;
  }

  return 1;
}

int mutt_is_text_part (BODY * b)
{
  int t = b->type;
  char *s = b->subtype;

  if ((WithCrypto & APPLICATION_PGP) && mutt_is_application_pgp (b))
    return 0;

  if (t == TYPETEXT)
    return 1;

  if (t == TYPEMESSAGE) {
    if (!ascii_strcasecmp ("delivery-status", s))
      return 1;
  }

  if ((WithCrypto & APPLICATION_PGP) && t == TYPEAPPLICATION) {
    if (!ascii_strcasecmp ("pgp-keys", s))
      return 1;
  }

  return 0;
}

void mutt_free_envelope (ENVELOPE ** p)
{
  if (!*p)
    return;
  rfc822_free_address (&(*p)->return_path);
  rfc822_free_address (&(*p)->from);
  rfc822_free_address (&(*p)->to);
  rfc822_free_address (&(*p)->cc);
  rfc822_free_address (&(*p)->bcc);
  rfc822_free_address (&(*p)->sender);
  rfc822_free_address (&(*p)->reply_to);
  rfc822_free_address (&(*p)->mail_followup_to);

  mem_free (&(*p)->list_post);
  mem_free (&(*p)->subject);
  /* real_subj is just an offset to subject and shouldn't be freed */
  mem_free (&(*p)->message_id);
  mem_free (&(*p)->supersedes);
  mem_free (&(*p)->date);
  mem_free (&(*p)->x_label);
  mem_free (&(*p)->organization);
#ifdef USE_NNTP
  mem_free (&(*p)->newsgroups);
  mem_free (&(*p)->xref);
  mem_free (&(*p)->followup_to);
  mem_free (&(*p)->x_comment_to);
#endif

  mutt_buffer_free (&(*p)->spam);
  mutt_free_list (&(*p)->references);
  mutt_free_list (&(*p)->in_reply_to);
  mutt_free_list (&(*p)->userhdrs);
  mem_free (p);
}

/* move all the headers from extra not present in base into base */
void mutt_merge_envelopes(ENVELOPE* base, ENVELOPE** extra)
{
  /* copies each existing element if necessary, and sets the element
  * to NULL in the source so that mutt_free_envelope doesn't leave us
  * with dangling pointers. */
#define MOVE_ELEM(h) if (!base->h) { base->h = (*extra)->h; (*extra)->h = NULL; }
  MOVE_ELEM(return_path);
  MOVE_ELEM(from);
  MOVE_ELEM(to);
  MOVE_ELEM(cc);
  MOVE_ELEM(bcc);
  MOVE_ELEM(sender);
  MOVE_ELEM(reply_to);
  MOVE_ELEM(mail_followup_to);
  MOVE_ELEM(list_post);
  MOVE_ELEM(message_id);
  MOVE_ELEM(supersedes);
  MOVE_ELEM(date);
  MOVE_ELEM(x_label);
  if (!base->refs_changed) {
    MOVE_ELEM(references);
  }
  if (!base->irt_changed) {
    MOVE_ELEM(in_reply_to);
  }
  /* real_subj is subordinate to subject */
  if (!base->subject) {
    base->subject = (*extra)->subject;
    base->real_subj = (*extra)->real_subj;
    (*extra)->subject = NULL;
    (*extra)->real_subj = NULL;
  }
  /* spam and user headers should never be hashed, and the new envelope may
   * have better values. Use new versions regardless. */
  mutt_buffer_free (&base->spam);
  mutt_free_list (&base->userhdrs);
  MOVE_ELEM(spam);
  MOVE_ELEM(userhdrs);
#undef MOVE_ELEM
  
  mutt_free_envelope(extra);
}

void _mutt_mktemp (char *s, const char *src, int line)
{

  snprintf (s, _POSIX_PATH_MAX, "%s/muttng-%s-%d-%d-%d-%x%x", NONULL (Tempdir),
            NONULL (Hostname), (int) getuid (), (int) getpid (), Counter++, 
            (unsigned int) rand(), (unsigned int) rand());
  debug_print (1, ("%s:%d: mutt_mktemp returns \"%s\".\n", src, line, s));
  unlink (s);
}

void mutt_free_alias (ALIAS ** p)
{
  ALIAS *t;

  while (*p) {
    t = *p;
    *p = (*p)->next;
    mem_free (&t->name);
    rfc822_free_address (&t->addr);
    mem_free (&t);
  }
}

/* collapse the pathname using ~ or = when possible */
void mutt_pretty_mailbox (char *s)
{
  char *p = s, *q = s;
  size_t len;
  url_scheme_t scheme;

  scheme = url_check_scheme (s);

#ifdef USE_IMAP
  if (scheme == U_IMAP || scheme == U_IMAPS) {
    imap_pretty_mailbox (s);
    return;
  }
#endif

  /* if s is an url, only collapse path component */
  if (scheme != U_UNKNOWN) {
    p = strchr (s, ':') + 1;
    if (!strncmp (p, "//", 2))
      q = strchr (p + 2, '/');
    if (!q)
      q = strchr (p, '\0');
    p = q;
  }

  /* first attempt to collapse the pathname */
  while (*p) {
    if (*p == '/' && p[1] == '/') {
      *q++ = '/';
      p += 2;
    }
    else if (p[0] == '/' && p[1] == '.' && p[2] == '/') {
      *q++ = '/';
      p += 3;
    }
    else
      *q++ = *p++;
  }
  *q = 0;

  if (str_ncmp (s, Maildir, (len = str_len (Maildir))) == 0 &&
      s[len] == '/') {
    *s++ = '=';
    memmove (s, s + len, str_len (s + len) + 1);
  }
  else if (str_ncmp (s, Homedir, (len = str_len (Homedir))) == 0 &&
           s[len] == '/') {
    *s++ = '~';
    memmove (s, s + len - 1, str_len (s + len - 1) + 1);
  }
}

void mutt_pretty_size (char *s, size_t len, long n)
{
  if (n == 0)
    strfcpy (s, "0K", len);
  else if (n < 10189)           /* 0.1K - 9.9K */
    snprintf (s, len, "%3.1fK", (n < 103) ? 0.1 : n / 1024.0);
  else if (n < 1023949) {       /* 10K - 999K */
    /* 51 is magic which causes 10189/10240 to be rounded up to 10 */
    snprintf (s, len, "%ldK", (n + 51) / 1024);
  }
  else if (n < 10433332)        /* 1.0M - 9.9M */
    snprintf (s, len, "%3.1fM", n / 1048576.0);
  else {                        /* 10M+ */

    /* (10433332 + 52428) / 1048576 = 10 */
    snprintf (s, len, "%ldM", (n + 52428) / 1048576);
  }
}

void mutt_expand_file_fmt (char *dest, size_t destlen, const char *fmt,
                           const char *src)
{
  char tmp[LONG_STRING];

  mutt_quote_filename (tmp, sizeof (tmp), src);
  mutt_expand_fmt (dest, destlen, fmt, tmp);
}

void mutt_expand_fmt (char *dest, size_t destlen, const char *fmt,
                      const char *src)
{
  const char *p;
  char *d;
  size_t slen;
  int found = 0;

  slen = str_len (src);
  destlen--;

  for (p = fmt, d = dest; destlen && *p; p++) {
    if (*p == '%') {
      switch (p[1]) {
      case '%':
        *d++ = *p++;
        destlen--;
        break;
      case 's':
        found = 1;
        strfcpy (d, src, destlen + 1);
        d += destlen > slen ? slen : destlen;
        destlen -= destlen > slen ? slen : destlen;
        p++;
        break;
      default:
        *d++ = *p;
        destlen--;
        break;
      }
    }
    else {
      *d++ = *p;
      destlen--;
    }
  }

  *d = '\0';

  if (!found && destlen > 0) {
    str_cat (dest, destlen, " ");
    str_cat (dest, destlen, src);
  }

}

/* return 0 on success, -1 on abort, 1 on error */
int mutt_check_overwrite (const char *attname, const char *path,
                          char *fname, size_t flen, int *append,
                          char **directory)
{
  int rc = 0;
  char tmp[_POSIX_PATH_MAX];
  struct stat st;

  strfcpy (fname, path, flen);
  if (access (fname, F_OK) != 0)
    return 0;
  if (stat (fname, &st) != 0)
    return -1;
  if (S_ISDIR (st.st_mode)) {
    if (directory) {
      switch (mutt_multi_choice
              (_("File is a directory, save under it? [(y)es, (n)o, (a)ll]"),
               _("yna"))) {
      case 3:                  /* all */
        str_replace (directory, fname);
        break;
      case 1:                  /* yes */
        mem_free (directory);
        break;
      case -1:                 /* abort */
        mem_free (directory);
        return -1;
      case 2:                  /* no */
        mem_free (directory);
        return 1;
      }
    }
    else
      if ((rc =
           mutt_yesorno (_("File is a directory, save under it?"),
                         M_YES)) != M_YES)
      return (rc == M_NO) ? 1 : -1;

    if (!attname || !attname[0]) {
      tmp[0] = 0;
      if (mutt_get_field (_("File under directory: "), tmp, sizeof (tmp),
                          M_FILE | M_CLEAR) != 0 || !tmp[0])
        return (-1);
      mutt_concat_path (fname, path, tmp, flen);
    }
    else
      mutt_concat_path (fname, path, mutt_basename (attname), flen);
  }

  if (*append == 0 && access (fname, F_OK) == 0) {
    switch (mutt_multi_choice
            (_("File exists, (o)verwrite, (a)ppend, or (c)ancel?"), _("oac")))
    {
    case -1:                   /* abort */
      return -1;
    case 3:                    /* cancel */
      return 1;

    case 2:                    /* append */
      *append = M_SAVE_APPEND;
      break;
    case 1:                    /* overwrite */
      *append = M_SAVE_OVERWRITE;
      break;
    }
  }
  return 0;
}

void mutt_save_path (char *d, size_t dsize, ADDRESS * a)
{
  if (a && a->mailbox) {
    strfcpy (d, a->mailbox, dsize);
    if (!option (OPTSAVEADDRESS)) {
      char *p;

      if ((p = strpbrk (d, "%@")))
        *p = 0;
    }
    str_tolower (d);
  }
  else
    *d = 0;
}

void mutt_safe_path (char *s, size_t l, ADDRESS * a)
{
  char *p;

  mutt_save_path (s, l, a);
  for (p = s; *p; p++)
    if (*p == '/' || ISSPACE (*p) || !IsPrint ((unsigned char) *p))
      *p = '_';
}

/* counts how many characters in s can be skipped while none of the
 * characters of c appears */
int mutt_skipchars (const char *s, const char *c)
{
  int ret = 0;
  const char *p = s;

  while (s && *s) {
    register const char *t = c;

    while (t && *t) {
      if (*t == *s)
        return (ret);
      t++;
    }
    ret++;
    s++;
  }
  return (str_len (p));
}

void mutt_FormatString (char *dest,     /* output buffer */
                        size_t destlen, /* output buffer len */
                        const char *src,        /* template string */
                        format_t * callback,    /* callback for processing */
                        unsigned long data,     /* callback data */
                        format_flag flags)
{                               /* callback flags */
  char prefix[SHORT_STRING], buf[LONG_STRING], *cp, *wptr = dest, ch;
  char ifstring[SHORT_STRING], elsestring[SHORT_STRING];
  size_t wlen, count, len, col, wid;

  prefix[0] = '\0';
  destlen--;                    /* save room for the terminal \0 */
  wlen = (flags & M_FORMAT_ARROWCURSOR && option (OPTARROWCURSOR)) ? 3 : 0;
  col = wlen;

  while (*src && wlen < destlen) {
    if (*src == '%') {
      if (*++src == '%') {
        *wptr++ = '%';
        wlen++;
        col++;
        src++;
        continue;
      }

      if (*src == '?') {
        flags |= M_FORMAT_OPTIONAL;
        src++;
      }
      else {
        flags &= ~M_FORMAT_OPTIONAL;

        /* eat the format string */
        cp = prefix;
        count = 0;
        while (count < sizeof (prefix) &&
               (isdigit ((unsigned char) *src) || *src == '.' || *src == '-'))
        {
          *cp++ = *src++;
          count++;
        }
        *cp = 0;
      }

      if (!*src)
        break;                  /* bad format */

      ch = *src++;              /* save the character to switch on */

      if (flags & M_FORMAT_OPTIONAL) {
        if (*src != '?')
          break;                /* bad format */
        src++;

        /* eat the `if' part of the string */
        cp = ifstring;
        count = 0;
        while (count < sizeof (ifstring) && *src && *src != '?'
               && *src != '&') {
          *cp++ = *src++;
          count++;
        }
        *cp = 0;

        /* eat the `else' part of the string (optional) */
        if (*src == '&')
          src++;                /* skip the & */
        cp = elsestring;
        count = 0;
        while (count < sizeof (elsestring) && *src && *src != '?') {
          *cp++ = *src++;
          count++;
        }
        *cp = 0;

        if (!*src)
          break;                /* bad format */

        src++;                  /* move past the trailing `?' */
      }

      /* handle generic cases first */
      if (ch == '>') {
        /* right justify to EOL */
        ch = *src++;            /* pad char */
        /* calculate space left on line.  if we've already written more data
           than will fit on the line, ignore the rest of the line */
        if (DrawFullLine || option (OPTSTATUSONTOP))
          count = (COLS < destlen ? COLS : destlen);
        else
          count = ((COLS - SW) < destlen ? (COLS - SW) : destlen);
        if (count > col) {
          count -= col;         /* how many columns left on this line */
          mutt_FormatString (buf, sizeof (buf), src, callback, data, flags);
          wid = str_len (buf);
          if (count > wid) {
            count -= wid;       /* how many chars to pad */
            memset (wptr, ch, count);
            wptr += count;
            col += count;
          }
          if (wid + wlen > destlen)
            len = destlen - wlen;
          else
            len = wid;
          memcpy (wptr, buf, len);
          wptr += len;
          wlen += len;
          col += mutt_strwidth (buf);
        }
        break;                  /* skip rest of input */
      }
      else if (ch == '|') {
        /* pad to EOL */
        ch = *src++;
        if (destlen > COLS)
          destlen = COLS;
        if (destlen > wlen) {
          count = destlen - wlen;
          memset (wptr, ch, count);
          wptr += count;
        }
        break;                  /* skip rest of input */
      }
      else {
        short tolower = 0;
        short nodots = 0;

        while (ch == '_' || ch == ':') {
          if (ch == '_')
            tolower = 1;
          else if (ch == ':')
            nodots = 1;

          ch = *src++;
        }

        /* use callback function to handle this case */
        src =
          callback (buf, sizeof (buf), ch, src, prefix, ifstring, elsestring,
                    data, flags);

        if (tolower)
          str_tolower (buf);
        if (nodots) {
          char *p = buf;

          for (; *p; p++)
            if (*p == '.')
              *p = '_';
        }

        if ((len = str_len (buf)) + wlen > destlen)
          len = (destlen - wlen > 0) ? (destlen - wlen) : 0;

        memcpy (wptr, buf, len);
        wptr += len;
        wlen += len;
        col += mutt_strwidth (buf);
      }
    }
    else if (*src == '\\') {
      if (!*++src)
        break;
      switch (*src) {
      case 'n':
        *wptr = '\n';
        break;
      case 't':
        *wptr = '\t';
        break;
      case 'r':
        *wptr = '\r';
        break;
      case 'f':
        *wptr = '\f';
        break;
      case 'v':
        *wptr = '\v';
        break;
      default:
        *wptr = *src;
        break;
      }
      src++;
      wptr++;
      wlen++;
      col++;
    }
    else {
      unsigned int bar = mutt_skipchars (src, "%\\");
      char *bar2 = mem_malloc (bar + 1);

      strfcpy (bar2, src, bar + 1);
      while (bar--) {
        *wptr++ = *src++;
        wlen++;
      }
      col += mutt_strwidth (bar2);
      mem_free (&bar2);
    }
  }
  *wptr = 0;

#if 0
  if (flags & M_FORMAT_MAKEPRINT) {
    /* Make sure that the string is printable by changing all non-printable
       chars to dots, or spaces for non-printable whitespace */
    for (cp = dest; *cp; cp++)
      if (!IsPrint (*cp) && !((flags & M_FORMAT_TREE) && (*cp <= M_TREE_MAX)))
        *cp = isspace ((unsigned char) *cp) ? ' ' : '.';
  }
#endif
}

/* This function allows the user to specify a command to read stdout from in
   place of a normal file.  If the last character in the string is a pipe (|),
   then we assume it is a commmand to run instead of a normal file. */
FILE *mutt_open_read (const char *path, pid_t * thepid)
{
  FILE *f;
  struct stat s;

  int len = str_len (path);

  if (path[len - 1] == '|') {
    /* read from a pipe */

    char *s = str_dup (path);

    s[len - 1] = 0;
    mutt_endwin (NULL);
    *thepid = mutt_create_filter (s, NULL, &f, NULL);
    mem_free (&s);
  }
  else {
    if (stat (path, &s) < 0)
      return (NULL);
    if (S_ISDIR (s.st_mode)) {
      errno = EINVAL;
      return (NULL);
    }
    f = fopen (path, "r");
    *thepid = -1;
  }
  return (f);
}

/* returns 0 if OK to proceed, -1 to abort, 1 to retry */
int mutt_save_confirm (const char *s, struct stat *st)
{
  char tmp[_POSIX_PATH_MAX];
  int ret = 0;
  int rc;
  int magic = 0;

  magic = mx_get_magic (s);

#ifdef USE_POP
  if (magic == M_POP) {
    mutt_error _("Can't save message to POP mailbox.");

    return 1;
  }
#endif

#ifdef USE_NNTP
  if (magic == M_NNTP) {
    mutt_error _("Can't save message to newsserver.");

    return 0;
  }
#endif

  if (magic > 0 && !mx_access (s, W_OK)) {
    if (option (OPTCONFIRMAPPEND) &&
        (!TrashPath || (str_cmp (s, TrashPath) != 0))) {
      /* if we're appending to the trash, there's no point in asking */
      snprintf (tmp, sizeof (tmp), _("Append messages to %s?"), s);
      if ((rc = mutt_yesorno (tmp, M_YES)) == M_NO)
        ret = 1;
      else if (rc == -1)
        ret = -1;
    }
  }

  if (stat (s, st) != -1) {
    if (magic == -1) {
      mutt_error (_("%s is not a mailbox!"), s);
      return 1;
    }
  }
  else {
#ifdef USE_IMAP
    if (magic != M_IMAP)
#endif /* execute the block unconditionally if we don't use imap */
    {
      st->st_mtime = 0;
      st->st_atime = 0;

      if (errno == ENOENT) {
        if (option (OPTCONFIRMCREATE)) {
          snprintf (tmp, sizeof (tmp), _("Create %s?"), s);
          if ((rc = mutt_yesorno (tmp, M_YES)) == M_NO)
            ret = 1;
          else if (rc == -1)
            ret = -1;
        }
      }
      else {
        mutt_perror (s);
        return 1;
      }
    }
  }

  CLEARLINE (LINES - 1);
  return (ret);
}

void mutt_display_sanitize (char *s)
{
  for (; *s; s++) {
    if (!IsPrint (*s))
      *s = '?';
  }
}

void mutt_sleep (short s)
{
  if (SleepTime > s)
    sleep (SleepTime);
  else if (s)
    sleep (s);
}

/* Decrease a file's modification time by 1 second */
time_t mutt_decrease_mtime (const char *f, struct stat *st)
{
  struct utimbuf utim;
  struct stat _st;
  time_t mtime;

  if (!st) {
    if (stat (f, &_st) == -1)
      return -1;
    st = &_st;
  }

  if ((mtime = st->st_mtime) == time (NULL)) {
    mtime -= 1;
    utim.actime = mtime;
    utim.modtime = mtime;
    utime (f, &utim);
  }

  return mtime;
}

/* sets mtime of 'to' to mtime of 'from' */
void mutt_set_mtime (const char* from, const char* to) {
  struct utimbuf utim;
  struct stat st;

  if (stat (from, &st) != -1) {
    utim.actime = st.st_mtime;
    utim.modtime = st.st_mtime;
    utime (to, &utim);
  }
}

const char *mutt_make_version (int full)
{
  static char vstring[STRING];

  if (full)
    snprintf (vstring, sizeof (vstring), "Mutt-ng %s-r%s (based "
              "on Mutt 1.5.11/%s)", MUTT_VERSION, MUTT_REVISION,
              ReleaseDate);
  else
    snprintf (vstring, sizeof (vstring), "mutt-ng/%s-r%s",
              MUTT_VERSION, MUTT_REVISION);
  return vstring;
}

void mutt_free_spam_list (SPAM_LIST ** list)
{
  SPAM_LIST *p;

  if (!list)
    return;
  while (*list) {
    p = *list;
    *list = (*list)->next;
    rx_free (&p->rx);
    mem_free(&p->template);
    mem_free(&p);
  }
}

int mutt_match_spam_list (const char *s, SPAM_LIST * l, char *text, int x)
{
  static regmatch_t *pmatch = NULL;
  static int nmatch = 0;
  int i, n, tlen;
  char *p;

  if (!s)
    return 0;

  tlen = 0;

  for (; l; l = l->next) {
    /* If this pattern needs more matches, expand pmatch. */
    if (l->nmatch > nmatch) {
      mem_realloc (&pmatch, l->nmatch * sizeof (regmatch_t));
      nmatch = l->nmatch;
    }

    /* Does this pattern match? */
    if (regexec
        (l->rx->rx, s, (size_t) l->nmatch, (regmatch_t *) pmatch,
         (int) 0) == 0) {
      debug_print (5, ("%s matches %s\n%d subst", s, l->rx->pattern, l->rx->rx->re_nsub));

      /* Copy template into text, with substitutions. */
      for (p = l->template; *p;) {
        if (*p == '%') {
          n = atoi (++p);       /* find pmatch index */
          while (isdigit ((unsigned char) *p))
            ++p;                /* skip subst token */
          for (i = pmatch[n].rm_so; (i < pmatch[n].rm_eo) && (tlen < x); i++)
            text[tlen++] = s[i];
        }
        else {
          text[tlen++] = *p++;
        }
      }
      text[tlen] = '\0';
      debug_print (5, ("\"%s\"\n", text));
      return 1;
    }
  }

  return 0;
}

int mutt_cmp_header (const HEADER * h1, const HEADER * h2) {
  if (h1 && h2) {
    if (h1->received != h2->received ||
        h1->date_sent != h2->date_sent ||
        h1->content->length != h2->content->length ||
        h1->lines != h2->lines ||
        h1->zhours != h2->zhours ||
        h1->zminutes != h2->zminutes ||
        h1->zoccident != h2->zoccident ||
        h1->mime != h2->mime ||
        !mutt_cmp_env (h1->env, h2->env) ||
        !mutt_cmp_body (h1->content, h2->content))
      return (0);
    else
      return (1);
  }
  else {
    if (h1 == NULL && h2 == NULL)
      return (1);
    else
      return (0);
  }
}

/* return 1 if address lists are strictly identical */
int mutt_cmp_addr (const ADDRESS * a, const ADDRESS * b)
{
  while (a && b) {
    if (str_cmp (a->mailbox, b->mailbox) ||
        str_cmp (a->personal, b->personal))
      return (0);

    a = a->next;
    b = b->next;
  }
  if (a || b)
    return (0);

  return (1);
}

int mutt_cmp_list (const LIST * a, const LIST * b)
{
  while (a && b) {
    if (str_cmp (a->data, b->data))
      return (0);

    a = a->next;
    b = b->next;
  }
  if (a || b)
    return (0);

  return (1);
}

int mutt_cmp_env (const ENVELOPE * e1, const ENVELOPE * e2)
{
  if (e1 && e2) {
    if (str_cmp (e1->message_id, e2->message_id) ||
        str_cmp (e1->subject, e2->subject) ||
        !mutt_cmp_list (e1->references, e2->references) ||
        !mutt_cmp_addr (e1->from, e2->from) ||
        !mutt_cmp_addr (e1->sender, e2->sender) ||
        !mutt_cmp_addr (e1->reply_to, e2->reply_to) ||
        !mutt_cmp_addr (e1->to, e2->to) ||
        !mutt_cmp_addr (e1->cc, e2->cc) ||
        !mutt_cmp_addr (e1->return_path, e2->return_path))
      return (0);
    else
      return (1);
  }
  else {
    if (e1 == NULL && e2 == NULL)
      return (1);
    else
      return (0);
  }
}

int mutt_cmp_param (const PARAMETER * p1, const PARAMETER * p2)
{
  while (p1 && p2) {
    if (str_cmp (p1->attribute, p2->attribute) ||
        str_cmp (p1->value, p2->value))
      return (0);

    p1 = p1->next;
    p2 = p2->next;
  }
  if (p1 || p2)
    return (0);

  return (1);
}

int mutt_cmp_body (const BODY * b1, const BODY * b2)
{
  if (b1->type != b2->type ||
      b1->encoding != b2->encoding ||
      str_cmp (b1->subtype, b2->subtype) ||
      str_cmp (b1->description, b2->description) ||
      !mutt_cmp_param (b1->parameter, b2->parameter) ||
      b1->length != b2->length)
    return (0);
  return (1);
}
