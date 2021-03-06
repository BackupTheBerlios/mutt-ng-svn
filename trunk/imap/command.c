/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-8 Michael R. Elkins <me@mutt.org>
 * Copyright (C) 1996-9 Brandon Long <blong@fiction.net>
 * Copyright (C) 1999-2005 Brendan Cully <brendan@kublai.com>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

/* command.c: routines for sending commands to an IMAP server and parsing
 *  responses */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "lib/mem.h"
#include "lib/intl.h"
#include "lib/debug.h"

#include "mutt.h"
#include "message.h"
#include "mx.h"
#include "ascii.h"
#include "imap_private.h"

#include <ctype.h>
#include <stdlib.h>

#define IMAP_CMD_BUFSIZE 512

/* forward declarations */
static void cmd_handle_fatal (IMAP_DATA * idata);
static int cmd_handle_untagged (IMAP_DATA * idata);
static void cmd_make_sequence (IMAP_DATA * idata);
static void cmd_parse_capabilities (IMAP_DATA * idata, char *s);
static void cmd_parse_expunge (IMAP_DATA * idata, const char *s);
static void cmd_parse_lsub (IMAP_DATA* idata, char* s);
static void cmd_parse_fetch (IMAP_DATA * idata, char *s);
static void cmd_parse_myrights (IMAP_DATA * idata, char *s);
static void cmd_parse_search (IMAP_DATA* idata, char* s);

static char *Capabilities[] = {
  "IMAP4",
  "IMAP4rev1",
  "STATUS",
  "ACL",
  "NAMESPACE",
  "AUTH=CRAM-MD5",
  "AUTH=GSSAPI",
  "AUTH=ANONYMOUS",
  "STARTTLS",
  "LOGINDISABLED",

  NULL
};

/* imap_cmd_start: Given an IMAP command, send it to the server.
 *   Currently a minor convenience, but helps to route all IMAP commands
 *   through a single interface. */
int imap_cmd_start (IMAP_DATA * idata, const char *cmd)
{
  char *out;
  int outlen;
  int rc;

  if (idata->status == IMAP_FATAL) {
    cmd_handle_fatal (idata);
    return IMAP_CMD_BAD;
  }

  cmd_make_sequence (idata);
  /* seq, space, cmd, \r\n\0 */
  outlen = str_len (idata->cmd.seq) + str_len (cmd) + 4;
  out = (char *) mem_malloc (outlen);
  snprintf (out, outlen, "%s %s\r\n", idata->cmd.seq, cmd);

  rc = mutt_socket_write (idata->conn, out);

  mem_free (&out);

  return (rc < 0) ? IMAP_CMD_BAD : 0;
}

/* imap_cmd_step: Reads server responses from an IMAP command, detects
 *   tagged completion response, handles untagged messages, can read
 *   arbitrarily large strings (using malloc, so don't make it _too_
 *   large!). */
int imap_cmd_step (IMAP_DATA * idata)
{
  IMAP_COMMAND *cmd = &idata->cmd;
  unsigned int len = 0;
  int c;

  if (idata->status == IMAP_FATAL) {
    cmd_handle_fatal (idata);
    return IMAP_CMD_BAD;
  }

  /* read into buffer, expanding buffer as necessary until we have a full
   * line */
  do {
    if (len == cmd->blen) {
      mem_realloc (&cmd->buf, cmd->blen + IMAP_CMD_BUFSIZE);
      cmd->blen = cmd->blen + IMAP_CMD_BUFSIZE;
      debug_print (3, ("grew buffer to %u bytes\n", cmd->blen));
    }

    if (len)
      len--;

    c = mutt_socket_readln (cmd->buf + len, cmd->blen - len, idata->conn);
    if (c <= 0) {
      debug_print (1, ("Error reading server response.\n"));
      /* cmd_handle_fatal (idata); */
      return IMAP_CMD_BAD;
    }

    len += c;
  }
  /* if we've read all the way to the end of the buffer, we haven't read a
   * full line (mutt_socket_readln strips the \r, so we always have at least
   * one character free when we've read a full line) */
  while (len == cmd->blen);

  /* don't let one large string make cmd->buf hog memory forever */
  if ((cmd->blen > IMAP_CMD_BUFSIZE) && (len <= IMAP_CMD_BUFSIZE)) {
    mem_realloc (&cmd->buf, IMAP_CMD_BUFSIZE);
    cmd->blen = IMAP_CMD_BUFSIZE;
    debug_print (3, ("shrank buffer to %u bytes\n", cmd->blen));
  }

  idata->lastread = time (NULL);

  /* handle untagged messages. The caller still gets its shot afterwards. */
  if (!ascii_strncmp (cmd->buf, "* ", 2) && cmd_handle_untagged (idata))
    return IMAP_CMD_BAD;

  /* server demands a continuation response from us */
  if (cmd->buf[0] == '+')
    return IMAP_CMD_RESPOND;

  /* tagged completion code */
  if (!ascii_strncmp (cmd->buf, cmd->seq, SEQLEN)) {
    imap_cmd_finish (idata);
    return imap_code (cmd->buf) ? IMAP_CMD_OK : IMAP_CMD_NO;
  }

  return IMAP_CMD_CONTINUE;
}

/* imap_code: returns 1 if the command result was OK, or 0 if NO or BAD */
int imap_code (const char *s)
{
  s += SEQLEN;
  SKIPWS (s);
  return (ascii_strncasecmp ("OK", s, 2) == 0);
}

/* imap_exec: execute a command, and wait for the response from the server.
 * Also, handle untagged responses.
 * Flags:
 *   IMAP_CMD_FAIL_OK: the calling procedure can handle failure. This is used
 *     for checking for a mailbox on append and login
 *   IMAP_CMD_PASS: command contains a password. Suppress logging.
 * Return 0 on success, -1 on Failure, -2 on OK Failure
 */
int imap_exec (IMAP_DATA * idata, const char *cmd, int flags)
{
  char *out;
  int outlen;
  int rc;

  if (!idata) {
    mutt_error (_("No mailbox is open."));
    mutt_sleep (1);
    return (-1);
  }

  if (idata->status == IMAP_FATAL) {
    cmd_handle_fatal (idata);
    return -1;
  }

  /* create sequence for command */
  cmd_make_sequence (idata);
  /* seq, space, cmd, \r\n\0 */
  outlen = str_len (idata->cmd.seq) + str_len (cmd) + 4;
  out = (char *) mem_malloc (outlen);
  snprintf (out, outlen, "%s %s\r\n", idata->cmd.seq, cmd);

  rc = mutt_socket_write_d (idata->conn, out,
                            flags & IMAP_CMD_PASS ? IMAP_LOG_PASS :
                            IMAP_LOG_CMD);
  mem_free (&out);

  if (rc < 0) {
    cmd_handle_fatal (idata);
    return -1;
  }

  do
    rc = imap_cmd_step (idata);
  while (rc == IMAP_CMD_CONTINUE);

  if (rc == IMAP_CMD_BAD) {
    if (imap_reconnect (idata->ctx) != 0) {
      return -1;
    }
    return 0;
  }

  if (rc == IMAP_CMD_NO && (flags & IMAP_CMD_FAIL_OK))
    return -2;

  if (rc != IMAP_CMD_OK) {
    if (flags & IMAP_CMD_FAIL_OK)
      return -2;

    debug_print (1, ("command failed: %s\n", idata->cmd.buf));
    return -1;
  }

  return 0;
}

/* imap_cmd_running: Returns whether an IMAP command is in progress. */
int imap_cmd_running (IMAP_DATA * idata)
{
  if (idata->cmd.state == IMAP_CMD_CONTINUE ||
      idata->cmd.state == IMAP_CMD_RESPOND)
    return 1;

  return 0;
}

/* imap_cmd_finish: Attempts to perform cleanup (eg fetch new mail if
 *   detected, do expunge). Called automatically by imap_cmd_step, but
 *   may be called at any time. Called by imap_check_mailbox just before
 *   the index is refreshed, for instance. */
void imap_cmd_finish (IMAP_DATA * idata)
{
  if (idata->status == IMAP_FATAL) {
    cmd_handle_fatal (idata);
    return;
  }

  if (!(idata->state == IMAP_SELECTED) || idata->ctx->closing)
    return;

  if (idata->reopen & IMAP_REOPEN_ALLOW) {
    int count = idata->newMailCount;

    if (!(idata->reopen & IMAP_EXPUNGE_PENDING) &&
        (idata->reopen & IMAP_NEWMAIL_PENDING)
        && count > idata->ctx->msgcount) {
      /* read new mail messages */
      debug_print (2, ("Fetching new mail\n"));
      /* check_status: curs_main uses imap_check_mailbox to detect
       *   whether the index needs updating */
      idata->check_status = IMAP_NEWMAIL_PENDING;
      imap_read_headers (idata, idata->ctx->msgcount, count - 1);
    }
    else if (idata->reopen & IMAP_EXPUNGE_PENDING) {
      debug_print (2, ("Expunging mailbox\n"));
      imap_expunge_mailbox (idata);
      /* Detect whether we've gotten unexpected EXPUNGE messages */
      if (idata->reopen & IMAP_EXPUNGE_PENDING &&
          !(idata->reopen & IMAP_EXPUNGE_EXPECTED))
        idata->check_status = IMAP_EXPUNGE_PENDING;
      idata->reopen &= ~(IMAP_EXPUNGE_PENDING | IMAP_NEWMAIL_PENDING |
                         IMAP_EXPUNGE_EXPECTED);
    }
  }

  idata->status = 0;
}

/* cmd_handle_fatal: when IMAP_DATA is in fatal state, do what we can */
static void cmd_handle_fatal (IMAP_DATA * idata)
{
  idata->status = IMAP_FATAL;

  if ((idata->state == IMAP_SELECTED) &&
      (idata->reopen & IMAP_REOPEN_ALLOW)) {
    /* mx_fastclose_mailbox (idata->ctx); */
    mutt_error (_("Mailbox closed"));
    mutt_sleep (1);
    idata->state = IMAP_DISCONNECTED;
    if (imap_reconnect (idata->ctx) != 0)
      mx_fastclose_mailbox (idata->ctx);
  }

  if (idata->state != IMAP_SELECTED) {
    idata->state = IMAP_DISCONNECTED;
    mutt_socket_close (idata->conn);
    idata->status = 0;
  }
}

/* cmd_handle_untagged: fallback parser for otherwise unhandled messages. */
static int cmd_handle_untagged (IMAP_DATA * idata)
{
  char *s;
  char *pn;
  int count;

  s = imap_next_word (idata->cmd.buf);

  if ((idata->state == IMAP_SELECTED) && isdigit ((unsigned char) *s)) {
    pn = s;
    s = imap_next_word (s);

    /* EXISTS and EXPUNGE are always related to the SELECTED mailbox for the
     * connection, so update that one.
     */
    if (ascii_strncasecmp ("EXISTS", s, 6) == 0) {
      debug_print (2, ("Handling EXISTS\n"));

      /* new mail arrived */
      count = atoi (pn);

      if (!(idata->reopen & IMAP_EXPUNGE_PENDING) &&
          count < idata->ctx->msgcount) {
        /* something is wrong because the server reported fewer messages
         * than we previously saw
         */
        mutt_error _("Fatal error.  Message count is out of sync!");

        idata->status = IMAP_FATAL;
        return -1;
      }
      /* at least the InterChange server sends EXISTS messages freely,
       * even when there is no new mail */
      else if (count == idata->ctx->msgcount)
        debug_print (3, ("superfluous EXISTS message.\n"));
      else {
        if (!(idata->reopen & IMAP_EXPUNGE_PENDING)) {
          debug_print (2, ("New mail in %s - %d messages total.\n", idata->mailbox, count));
          idata->reopen |= IMAP_NEWMAIL_PENDING;
        }
        idata->newMailCount = count;
      }
    }
    /* pn vs. s: need initial seqno */
    else if (ascii_strncasecmp ("EXPUNGE", s, 7) == 0)
      cmd_parse_expunge (idata, pn);
    else if (ascii_strncasecmp ("FETCH", s, 5) == 0)
      cmd_parse_fetch (idata, pn);
  }
  else if (ascii_strncasecmp ("CAPABILITY", s, 10) == 0)
    cmd_parse_capabilities (idata, s);
  else if (ascii_strncasecmp ("LSUB", s, 4) == 0)
    cmd_parse_lsub (idata, s);
  else if (ascii_strncasecmp ("MYRIGHTS", s, 8) == 0)
    cmd_parse_myrights (idata, s);
  else if (ascii_strncasecmp ("SEARCH", s, 6) == 0)
    cmd_parse_search (idata, s);
  else if (ascii_strncasecmp ("BYE", s, 3) == 0) {
    debug_print (2, ("Handling BYE\n"));

    /* check if we're logging out */
    if (idata->status == IMAP_BYE)
      return 0;

    /* server shut down our connection */
    s += 3;
    SKIPWS (s);
    mutt_error ("%s", s);
    mutt_sleep (2);
    cmd_handle_fatal (idata);
    return -1;
  }
  else if (option (OPTIMAPSERVERNOISE)
           && (ascii_strncasecmp ("NO", s, 2) == 0)) {
    debug_print (2, ("Handling untagged NO\n"));

    /* Display the warning message from the server */
    mutt_error ("%s", s + 3);
    mutt_sleep (2);
  }

  return 0;
}

/* This should be optimised (eg with a tree or hash) */
static int uid2msgno (IMAP_DATA* idata, unsigned int uid) {
  int i;

  for (i = 0; i < idata->ctx->msgcount; i++) {
    HEADER* h = idata->ctx->hdrs[i];
    if (HEADER_DATA(h)->uid == uid)
      return i;
  }

 return -1;
}

/* cmd_parse_search: store SEARCH response for later use */
static void cmd_parse_search (IMAP_DATA* idata, char* s) {
  unsigned int uid;
  int msgno;

  debug_print (2, ("Handling SEARCH\n"));

  while ((s = imap_next_word (s)) && *s != '\0') {
    uid = atoi (s);
    msgno = uid2msgno (idata, uid);
    
    if (msgno >= 0)
      idata->ctx->hdrs[uid2msgno (idata, uid)]->matched = 1;
  }
}

/* cmd_make_sequence: make a tag suitable for starting an IMAP command */
static void cmd_make_sequence (IMAP_DATA * idata)
{
  snprintf (idata->cmd.seq, sizeof (idata->cmd.seq), "a%04u", idata->seqno++);

  if (idata->seqno > 9999)
    idata->seqno = 0;
}

/* cmd_parse_capabilities: set capability bits according to CAPABILITY
 *   response */
static void cmd_parse_capabilities (IMAP_DATA * idata, char *s)
{
  int x;

  debug_print (2, ("Handling CAPABILITY\n"));

  s = imap_next_word (s);
  mem_free (&idata->capstr);
  idata->capstr = str_dup (s);

  memset (idata->capabilities, 0, sizeof (idata->capabilities));

  while (*s) {
    for (x = 0; x < CAPMAX; x++)
      if (imap_wordcasecmp (Capabilities[x], s) == 0) {
        mutt_bit_set (idata->capabilities, x);
        break;
      }
    s = imap_next_word (s);
  }
}

/* cmd_parse_expunge: mark headers with new sequence ID and mark idata to
 *   be reopened at our earliest convenience */
static void cmd_parse_expunge (IMAP_DATA * idata, const char *s)
{
  int expno, cur;
  HEADER *h;

  debug_print (2, ("Handling EXPUNGE\n"));

  expno = atoi (s);

  /* walk headers, zero seqno of expunged message, decrement seqno of those
   * above. Possibly we could avoid walking the whole list by resorting
   * and guessing a good starting point, but I'm guessing the resort would
   * nullify the gains */
  for (cur = 0; cur < idata->ctx->msgcount; cur++) {
    h = idata->ctx->hdrs[cur];

    if (h->index + 1 == expno)
      h->index = -1;
    else if (h->index + 1 > expno)
      h->index--;
  }

  idata->reopen |= IMAP_EXPUNGE_PENDING;
}

/* cmd_parse_fetch: Load fetch response into IMAP_DATA. Currently only
 *   handles unanticipated FETCH responses, and only FLAGS data. We get
 *   these if another client has changed flags for a mailbox we've selected.
 *   Of course, a lot of code here duplicates code in message.c. */
static void cmd_parse_fetch (IMAP_DATA * idata, char *s)
{
  int msgno, cur;
  HEADER *h = NULL;

  debug_print (2, ("Handling FETCH\n"));

  msgno = atoi (s);

  if (msgno <= idata->ctx->msgcount)
    /* see cmd_parse_expunge */
    for (cur = 0; cur < idata->ctx->msgcount; cur++) {
      h = idata->ctx->hdrs[cur];

      if (!h)
        break;

      if (h->active && h->index + 1 == msgno) {
        debug_print (2, ("Message UID %d updated\n", HEADER_DATA (h)->uid));
        break;
      }

      h = NULL;
    }

  if (!h) {
    debug_print (1, ("FETCH response ignored for this message\n"));
    return;
  }

  /* skip FETCH */
  s = imap_next_word (s);
  s = imap_next_word (s);

  if (*s != '(') {
    debug_print (1, ("Malformed FETCH response\n"));
    return;
  }
  s++;

  if (ascii_strncasecmp ("FLAGS", s, 5) != 0) {
    debug_print (2, ("Only handle FLAGS updates\n"));
    return;
  }

  /* If server flags could conflict with mutt's flags, reopen the mailbox. */
  if (h->changed)
    idata->reopen |= IMAP_EXPUNGE_PENDING;
  else {
    imap_set_flags (idata, h, s);
    idata->check_status = IMAP_FLAGS_PENDING;
  }
}

static void cmd_parse_lsub (IMAP_DATA* idata, char* s) {
  char buf[STRING];
  char errstr[STRING];
  BUFFER err, token;
  ciss_url_t url;
  char *ep;

  if (!option (OPTIMAPCHECKSUBSCRIBED))
    return;

  s = imap_next_word (s); /* flags */

  if (*s != '(') {
    debug_print (1, ("Bad LSUB response\n"));
    return;
  }

  s++;
  ep = s;
  for (ep = s; *ep && *ep != ')'; ep++);
  do {
    if (!ascii_strncasecmp (s, "\\NoSelect", 9))
      return;
    while (s < ep && *s != ' ' && *s != ')')
      s++;
    if (*s == ' ')
      s++;
  } while (s != ep);

  s = imap_next_word (s); /* delim */
  s = imap_next_word (s); /* name */

  if (s) {
    imap_unmunge_mbox_name (s);
    debug_print (2, ("Subscribing to %s\n", s));

    strfcpy (buf, "mailboxes \"", sizeof (buf));
    mutt_account_tourl (&idata->conn->account, &url);
    url.path = s;
    if (!str_cmp (url.user, ImapUser))
      url.user = NULL;
    url_ciss_tostring (&url, buf + 11, sizeof (buf) - 10, 0);
    str_cat (buf, sizeof (buf), "\"");
    memset (&token, 0, sizeof (token));
    err.data = errstr;
    err.dsize = sizeof (errstr);
    if (mutt_parse_rc_line (buf, &token, &err))
      debug_print (1, ("Error adding subscribed mailbox: %s\n", errstr));
    mem_free (&token.data);
  }
  else
    debug_print (1, ("Bad LSUB response\n"));
}

/* cmd_parse_myrights: set rights bits according to MYRIGHTS response */
static void cmd_parse_myrights (IMAP_DATA * idata, char *s)
{
  debug_print (2, ("Handling MYRIGHTS\n"));

  s = imap_next_word (s);
  s = imap_next_word (s);

  /* zero out current rights set */
  memset (idata->rights, 0, sizeof (idata->rights));

  while (*s && !isspace ((unsigned char) *s)) {
    switch (*s) {
    case 'l':
      mutt_bit_set (idata->rights, ACL_LOOKUP);
      break;
    case 'r':
      mutt_bit_set (idata->rights, ACL_READ);
      break;
    case 's':
      mutt_bit_set (idata->rights, ACL_SEEN);
      break;
    case 'w':
      mutt_bit_set (idata->rights, ACL_WRITE);
      break;
    case 'i':
      mutt_bit_set (idata->rights, ACL_INSERT);
      break;
    case 'p':
      mutt_bit_set (idata->rights, ACL_POST);
      break;
    case 'c':
      mutt_bit_set (idata->rights, ACL_CREATE);
      break;
    case 'd':
      mutt_bit_set (idata->rights, ACL_DELETE);
      break;
    case 'a':
      mutt_bit_set (idata->rights, ACL_ADMIN);
      break;
    }
    s++;
  }
}
