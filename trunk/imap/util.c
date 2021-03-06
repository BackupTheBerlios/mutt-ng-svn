/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-8 Michael R. Elkins <me@mutt.org>
 * Copyright (C) 1996-9 Brandon Long <blong@fiction.net>
 * Copyright (C) 1999-2002 Brendan Cully <brendan@kublai.com>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

/* general IMAP utility functions */

#include "config.h"

#include "mutt.h"
#include "mx.h"                 /* for M_IMAP */
#include "ascii.h"
#include "url.h"
#include "imap_private.h"
#include "mutt_ssl.h"

#include "lib/mem.h"
#include "lib/intl.h"
#include "lib/debug.h"

#include <stdlib.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <netdb.h>
#include <netinet/in.h>

#include <errno.h>

/* -- public functions -- */

/* imap_expand_path: IMAP implementation of mutt_expand_path. Rewrite
 *   an IMAP path in canonical and absolute form.
 * Inputs: a buffer containing an IMAP path, and the number of bytes in
 *   that buffer.
 * Outputs: The buffer is rewritten in place with the canonical IMAP path.
 * Returns 0 on success, or -1 if imap_parse_path chokes or url_ciss_tostring
 *   fails, which it might if there isn't enough room in the buffer. */
int imap_expand_path (char *path, size_t len)
{
  IMAP_MBOX mx;
  ciss_url_t url;
  int rc;

  if (imap_parse_path (path, &mx) < 0)
    return -1;

  mutt_account_tourl (&mx.account, &url);
  url.path = mx.mbox;

  rc = url_ciss_tostring (&url, path, len, U_DECODE_PASSWD);
  mem_free (&mx.mbox);

  return rc;
}

/* imap_parse_path: given an IMAP mailbox name, return host, port
 *   and a path IMAP servers will recognise.
 * mx.mbox is malloc'd, caller must free it */
int imap_parse_path (const char *path, IMAP_MBOX * mx)
{
  static unsigned short ImapPort = 0;
  static unsigned short ImapsPort = 0;
  struct servent *service;
  ciss_url_t url;
  char *c;

  if (!ImapPort) {
    service = getservbyname ("imap", "tcp");
    if (service)
      ImapPort = ntohs (service->s_port);
    else
      ImapPort = IMAP_PORT;
    debug_print (3, ("Using default IMAP port %d\n", ImapPort));
  }
  if (!ImapsPort) {
    service = getservbyname ("imaps", "tcp");
    if (service)
      ImapsPort = ntohs (service->s_port);
    else
      ImapsPort = IMAP_SSL_PORT;
    debug_print (3, ("Using default IMAPS port %d\n", ImapsPort));
  }

  /* Defaults */
  mx->account.flags = 0;
  mx->account.port = ImapPort;
  mx->account.type = M_ACCT_TYPE_IMAP;

  c = str_dup (path);
  url_parse_ciss (&url, c);

  if (!(url.scheme == U_IMAP || url.scheme == U_IMAPS) ||
      mutt_account_fromurl (&mx->account, &url) < 0 || !*mx->account.host) {
    mem_free (&c);
    return -1;
  }

  mx->mbox = str_dup (url.path);

  if (url.scheme == U_IMAPS)
    mx->account.flags |= M_ACCT_SSL;

  mem_free (&c);

  if ((mx->account.flags & M_ACCT_SSL) && !(mx->account.flags & M_ACCT_PORT))
    mx->account.port = ImapsPort;

  return 0;
}

/* imap_pretty_mailbox: called by mutt_pretty_mailbox to make IMAP paths
 *   look nice. */
void imap_pretty_mailbox (char *path)
{
  IMAP_MBOX home, target;
  ciss_url_t url;
  char *delim;
  int tlen;
  int hlen = 0;
  char home_match = 0;

  if (imap_parse_path (path, &target) < 0)
    return;

  tlen = str_len (target.mbox);
  /* check whether we can do '=' substitution */
  if (mx_get_magic (Maildir) == M_IMAP && !imap_parse_path (Maildir, &home)) {
    hlen = str_len (home.mbox);
    if (tlen && mutt_account_match (&home.account, &target.account) &&
        !str_ncmp (home.mbox, target.mbox, hlen)) {
      if (!hlen)
        home_match = 1;
      else
        for (delim = ImapDelimChars; *delim != '\0'; delim++)
          if (target.mbox[hlen] == *delim)
            home_match = 1;
    }
    mem_free (&home.mbox);
  }

  /* do the '=' substitution */
  if (home_match) {
    *path++ = '=';
    /* copy remaining path, skipping delimiter */
    if (!hlen)
      hlen = -1;
    memcpy (path, target.mbox + hlen + 1, tlen - hlen - 1);
    path[tlen - hlen - 1] = '\0';
  }
  else {
    mutt_account_tourl (&target.account, &url);
    url.path = target.mbox;
    /* FIXME: That hard-coded constant is bogus. But we need the actual
     *   size of the buffer from mutt_pretty_mailbox. And these pretty
     *   operations usually shrink the result. Still... */
    url_ciss_tostring (&url, path, 1024, 0);
  }

  mem_free (&target.mbox);
}

/* -- library functions -- */

/* imap_continue: display a message and ask the user if she wants to
 *   go on. */
int imap_continue (const char *msg, const char *resp)
{
  imap_error (msg, resp);
  return mutt_yesorno (_("Continue?"), 0);
}

/* imap_error: show an error and abort */
void imap_error (const char *where, const char *msg)
{
  mutt_error ("%s [%s]\n", where, msg);
  mutt_sleep (2);
}

/* imap_new_idata: Allocate and initialise a new IMAP_DATA structure.
 *   Returns NULL on failure (no mem) */
IMAP_DATA *imap_new_idata (void)
{
  return mem_calloc (1, sizeof (IMAP_DATA));
}

/* imap_free_idata: Release and clear storage in an IMAP_DATA structure. */
void imap_free_idata (IMAP_DATA ** idata)
{
  if (!idata)
    return;

  mem_free (&(*idata)->capstr);
  mutt_free_list (&(*idata)->flags);
  mem_free (&((*idata)->cmd.buf));
  mem_free (idata);
}

/*
 * Fix up the imap path.  This is necessary because the rest of mutt
 * assumes a hierarchy delimiter of '/', which is not necessarily true
 * in IMAP.  Additionally, the filesystem converts multiple hierarchy
 * delimiters into a single one, ie "///" is equal to "/".  IMAP servers
 * are not required to do this.
 * Moreover, IMAP servers may dislike the path ending with the delimiter.
 */
char *imap_fix_path (IMAP_DATA * idata, char *mailbox, char *path,
                     size_t plen)
{
  int x = 0;

  if (!mailbox || !*mailbox) {
    strfcpy (path, "INBOX", plen);
    return path;
  }

  while (mailbox && *mailbox && (x < (plen - 1))) {
    if ((*mailbox == '/') || (*mailbox == idata->delim)) {
      while ((*mailbox == '/') || (*mailbox == idata->delim))
        mailbox++;
      path[x] = idata->delim;
    }
    else {
      path[x] = *mailbox;
      mailbox++;
    }
    x++;
  }
  if (x && path[--x] != idata->delim)
    x++;
  path[x] = '\0';
  return path;
}

/* imap_get_literal_count: write number of bytes in an IMAP literal into
 *   bytes, return 0 on success, -1 on failure. */
int imap_get_literal_count (const char *buf, long *bytes)
{
  char *pc;
  char *pn;

  if (!(pc = strchr (buf, '{')))
    return (-1);
  pc++;
  pn = pc;
  while (isdigit ((unsigned char) *pc))
    pc++;
  *pc = 0;
  *bytes = atoi (pn);
  return (0);
}

/* imap_get_qualifier: in a tagged response, skip tag and status for
 *   the qualifier message. Used by imap_copy_message for TRYCREATE */
char *imap_get_qualifier (char *buf)
{
  char *s = buf;

  /* skip tag */
  s = imap_next_word (s);
  /* skip OK/NO/BAD response */
  s = imap_next_word (s);

  return s;
}

/* imap_next_word: return index into string where next IMAP word begins */
char *imap_next_word (char *s)
{
  int quoted = 0;

  while (*s) {
    if (*s == '\\') {
      s++;
      if (*s)
        s++;
      continue;
    }
    if (*s == '\"')
      quoted = quoted ? 0 : 1;
    if (!quoted && ISSPACE (*s))
      break;
    s++;
  }

  SKIPWS (s);
  return s;
}

/* imap_parse_date: date is of the form: DD-MMM-YYYY HH:MM:SS +ZZzz */
time_t imap_parse_date (char *s)
{
  struct tm t;
  time_t tz;

  t.tm_mday = (s[0] == ' ' ? s[1] - '0' : (s[0] - '0') * 10 + (s[1] - '0'));
  s += 2;
  if (*s != '-')
    return 0;
  s++;
  t.tm_mon = mutt_check_month (s);
  s += 3;
  if (*s != '-')
    return 0;
  s++;
  t.tm_year =
    (s[0] - '0') * 1000 + (s[1] - '0') * 100 + (s[2] - '0') * 10 + (s[3] -
                                                                    '0') -
    1900;
  s += 4;
  if (*s != ' ')
    return 0;
  s++;

  /* time */
  t.tm_hour = (s[0] - '0') * 10 + (s[1] - '0');
  s += 2;
  if (*s != ':')
    return 0;
  s++;
  t.tm_min = (s[0] - '0') * 10 + (s[1] - '0');
  s += 2;
  if (*s != ':')
    return 0;
  s++;
  t.tm_sec = (s[0] - '0') * 10 + (s[1] - '0');
  s += 2;
  if (*s != ' ')
    return 0;
  s++;

  /* timezone */
  tz = ((s[1] - '0') * 10 + (s[2] - '0')) * 3600 +
    ((s[3] - '0') * 10 + (s[4] - '0')) * 60;
  if (s[0] == '+')
    tz = -tz;

  return (mutt_mktime (&t, 0) + tz);
}

/* imap_qualify_path: make an absolute IMAP folder target, given IMAP_MBOX
 *   and relative path. */
void imap_qualify_path (char *dest, size_t len, IMAP_MBOX * mx, char *path)
{
  ciss_url_t url;

  mutt_account_tourl (&mx->account, &url);
  url.path = path;

  url_ciss_tostring (&url, dest, len, 0);
}


/* imap_quote_string: quote string according to IMAP rules:
 *   surround string with quotes, escape " and \ with \ */
void imap_quote_string (char *dest, size_t dlen, const char *src)
{
  char quote[] = "\"\\", *pt;
  const char *s;

  pt = dest;
  s = src;

  *pt++ = '"';
  /* save room for trailing quote-char */
  dlen -= 2;

  for (; *s && dlen; s++) {
    if (strchr (quote, *s)) {
      dlen -= 2;
      if (!dlen)
        break;
      *pt++ = '\\';
      *pt++ = *s;
    }
    else {
      *pt++ = *s;
      dlen--;
    }
  }
  *pt++ = '"';
  *pt = 0;
}

/* imap_unquote_string: equally stupid unquoting routine */
void imap_unquote_string (char *s)
{
  char *d = s;

  if (*s == '\"')
    s++;
  else
    return;

  while (*s) {
    if (*s == '\"') {
      *d = '\0';
      return;
    }
    if (*s == '\\') {
      s++;
    }
    if (*s) {
      *d = *s;
      d++;
      s++;
    }
  }
  *d = '\0';
}

/*
 * Quoting and UTF-7 conversion
 */

void imap_munge_mbox_name (char *dest, size_t dlen, const char *src)
{
  char *buf;

  buf = str_dup (src);
  imap_utf7_encode (&buf);

  imap_quote_string (dest, dlen, buf);

  mem_free (&buf);
}

void imap_unmunge_mbox_name (char *s)
{
  char *buf;

  imap_unquote_string (s);

  buf = str_dup (s);
  if (buf) {
    imap_utf7_decode (&buf);
    strncpy (s, buf, str_len (s));
  }

  mem_free (&buf);
}

/* imap_wordcasecmp: find word a in word list b */
int imap_wordcasecmp (const char *a, const char *b)
{
  char tmp[SHORT_STRING];
  char *s = (char *) b;
  int i;

  tmp[SHORT_STRING - 1] = 0;
  for (i = 0; i < SHORT_STRING - 2; i++, s++) {
    if (!*s || ISSPACE (*s)) {
      tmp[i] = 0;
      break;
    }
    tmp[i] = *s;
  }
  tmp[i + 1] = 0;

  return ascii_strcasecmp (a, tmp);
}

/* 
 * Imap keepalive: poll the current folder to keep the
 * connection alive.
 * 
 */

static RETSIGTYPE alrm_handler (int sig)
{
  /* empty */
}

void imap_keepalive (void)
{
  CONNECTION *conn;
  CONTEXT *ctx = NULL;
  IMAP_DATA *idata;

  conn = mutt_socket_head ();
  while (conn) {
    if (conn->account.type == M_ACCT_TYPE_IMAP) {
      idata = (IMAP_DATA *) conn->data;

      if (idata->state >= IMAP_AUTHENTICATED
          && time (NULL) >= idata->lastread + ImapKeepalive) {
        if (idata->ctx)
          ctx = idata->ctx;
        else {
          ctx = mem_calloc (1, sizeof (CONTEXT));
          ctx->data = idata;
        }
        imap_check_mailbox (ctx, NULL, 1);
        if (!idata->ctx)
          mem_free (&ctx);
      }
    }

    conn = conn->next;
  }
}

int imap_wait_keepalive (pid_t pid)
{
  struct sigaction oldalrm;
  struct sigaction act;
  sigset_t oldmask;
  int rc;

  short imap_passive = option (OPTIMAPPASSIVE);
  int imap_askreconnect = quadoption (OPT_IMAPRECONNECT);

  set_option (OPTIMAPPASSIVE);
  set_option (OPTKEEPQUIET);
  set_quadoption (OPT_IMAPRECONNECT, M_NO);

  sigprocmask (SIG_SETMASK, NULL, &oldmask);

  sigemptyset (&act.sa_mask);
  act.sa_handler = alrm_handler;
#ifdef SA_INTERRUPT
  act.sa_flags = SA_INTERRUPT;
#else
  act.sa_flags = 0;
#endif

  sigaction (SIGALRM, &act, &oldalrm);

  alarm (ImapKeepalive);
  while (waitpid (pid, &rc, 0) < 0 && errno == EINTR) {
    alarm (0);                  /* cancel a possibly pending alarm */
    imap_keepalive ();
    alarm (ImapKeepalive);
  }

  alarm (0);                    /* cancel a possibly pending alarm */

  sigaction (SIGALRM, &oldalrm, NULL);
  sigprocmask (SIG_SETMASK, &oldmask, NULL);

  unset_option (OPTKEEPQUIET);
  if (!imap_passive)
    unset_option (OPTIMAPPASSIVE);
  set_quadoption (OPT_IMAPRECONNECT, imap_askreconnect);

  return rc;
}

/* Allow/disallow re-opening a folder upon expunge. */

void imap_allow_reopen (CONTEXT * ctx)
{
  if (ctx && ctx->magic == M_IMAP && CTX_DATA->ctx == ctx)
    CTX_DATA->reopen |= IMAP_REOPEN_ALLOW;
}

void imap_disallow_reopen (CONTEXT * ctx)
{
  if (ctx && ctx->magic == M_IMAP && CTX_DATA->ctx == ctx)
    CTX_DATA->reopen &= ~IMAP_REOPEN_ALLOW;
}
