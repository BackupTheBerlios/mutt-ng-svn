/*
 * Copyright notice from original mutt:
 * [none]
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */
#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "mutt.h"

#include "lib/mem.h"
#include "lib/intl.h"
#include "lib/str.h"

#include <errno.h>
#include <auth-client.h>
#include <libesmtp.h>

static char authpass[STRING] = "";

#define FAIL() \
  do { \
    ret = -1; \
    goto Done; \
  } while (0)
#define MSGFAIL(msg) \
  do { \
    mutt_error("%s", msg); \
    FAIL(); \
  } while (0)
#define LIBCFAIL(msg) \
  do { \
    mutt_error("%s: %s", msg, strerror(errno)); \
    FAIL(); \
  } while (0)
#define SMTPFAIL(msg) \
  do { \
    _mutt_libesmtp_perror(msg); \
    FAIL(); \
  } while (0)

/*
 * _mutt_libesmtp_ensure_init
 *   Make sure the libESMTP support in mutt is initialized at some time.
 */
static void _mutt_libesmtp_ensure_init ()
{
  static int libesmtp_init = 0;

  if (!libesmtp_init) {
    if (SmtpAuthUser)
      auth_client_init ();
    libesmtp_init = 1;
  }
}

/*
 * _mutt_libesmtp_perror
 *   Prints 'msg', a colon, and then a string representation of the
 *   libesmtp errno as a mutt error.
 */
static void _mutt_libesmtp_perror (const char *msg)
{
  char buf[512];

  mutt_error ("%s: %s", msg,
              smtp_strerror (smtp_errno (), buf, sizeof (buf)));
}

/*
 * _mutt_libesmtp_add_recipients
 *   Adds every address in 'addr' as a recipient to the smtp message
 *   'message'.  Note that this does not mean that they will necessarily
 *   show up in the mail headers (e.g., when bcc'ing).  Returns 0 upon
 *   success, -1 upon failure (and prints an error message).
 *
 *   Very similar to sendlib.c::add_args
 */
static int
_mutt_libesmtp_add_recipients (smtp_message_t message, ADDRESS * addr)
{
  int ret = 0;

  for (; addr; addr = addr->next) {
    /* weed out group mailboxes, since those are for display only */
    if (addr->mailbox && !addr->group) {
      if (!smtp_add_recipient (message, addr->mailbox))
        SMTPFAIL ("smtp_add_recipient");
    }
  }

Done:
  return ret;
}

static int
_mutt_libesmtp_auth_interact (auth_client_request_t request,
                              char **result, int fields, void *arg)
{
  int i;

  for (i = 0; i < fields; i++) {
    if (request[i].flags & AUTH_USER) {
      result[i] = SmtpAuthUser;
    }
    else if (request[i].flags & AUTH_PASS) {
      if (SmtpAuthPass) {
        result[i] = SmtpAuthPass;
      }
      else {
        if (authpass[0] == '\0') {
          char prompt[STRING];

          snprintf (prompt, sizeof (prompt), "%s%s: ", request[i].prompt,
                    (request[i].
                     flags & AUTH_CLEARTEXT) ? " (not encrypted)" : "");
          mutt_get_password (prompt, authpass, sizeof (authpass));
        }
        result[i] = authpass;
      }
    }
  }

  return 1;
}

#define BUFLEN 8192

static const char *_mutt_libesmtp_messagefp_cb (void **buf, int *len,
                                                void *arg)
{
  int octets;

  if (*buf == NULL)
    *buf = malloc (BUFLEN);

  if (len == NULL) {
    rewind ((FILE *) arg);
    return NULL;
  }

  if (fgets (*buf, BUFLEN - 2, (FILE *) arg) == NULL) {
    octets = 0;
  }
  else {
    char *p = strchr (*buf, '\0');

    if (p[-1] == '\n' && p[-2] != '\r') {
      strcpy (p - 1, "\r\n");
      p++;
    }
    octets = p - (char *) *buf;
  }

  *len = octets;
  return *buf;
}

/*
 * mutt_invoke_libesmtp
 *   Sends a mail message to the provided recipients using libesmtp.
 *   Returns 0 upon success, -1 upon failure (and prints an error
 *   message).
 */
int mutt_invoke_libesmtp (ADDRESS * from,       /* the sender */
                          ADDRESS * to, ADDRESS * cc, ADDRESS * bcc,    /* recips */
                          const char *msg,      /* file containing message */
                          int eightbit)
{                               /* message contains 8bit chars */
  int ret = 0;                  /* return value, default = success */
  smtp_session_t session;
  smtp_message_t message;
  char *hostportstr = NULL;
  size_t hostportlen;
  FILE *fp = NULL;
  auth_context_t authctx = NULL;
  const smtp_status_t *status;

  _mutt_libesmtp_ensure_init ();

  if ((session = smtp_create_session ()) == NULL)
    SMTPFAIL ("smtp_create_session");

  /* Create hostname:port string and tell libesmtp */
  /* len = SmtpHost len + colon + max port (65536 => 5 chars) + terminator */
  hostportlen = mutt_strlen (SmtpHost) + 7;
  hostportstr = safe_malloc (hostportlen);
  snprintf (hostportstr, hostportlen, "%s:%d", SmtpHost, SmtpPort);
  if (!smtp_set_server (session, hostportstr))
    SMTPFAIL ("smtp_set_server");

  if (SmtpAuthUser) {
    if ((authctx = auth_create_context ()) == NULL)
      MSGFAIL ("auth_create_context failed");
    auth_set_mechanism_flags (authctx, AUTH_PLUGIN_PLAIN, 0);
    auth_set_interact_cb (authctx, _mutt_libesmtp_auth_interact, NULL);

    if (!smtp_auth_set_context (session, authctx))
      SMTPFAIL ("smtp_auth_set_context");
  }

  if ((message = smtp_add_message (session)) == NULL)
    SMTPFAIL ("smtp_add_message");
  /*  Initialize envelope sender */
  if (!smtp_set_reverse_path (message, from->mailbox))
    SMTPFAIL ("smtp_set_reverse_path");

  if ((fp = fopen (msg, "r")) == NULL)
    LIBCFAIL ("fopen");
  if (!smtp_set_messagecb (message, _mutt_libesmtp_messagefp_cb, fp))
    SMTPFAIL ("smtp_set_messagecb");
  if (_mutt_libesmtp_add_recipients (message, to))
    FAIL ();
  if (_mutt_libesmtp_add_recipients (message, cc))
    FAIL ();
  if (_mutt_libesmtp_add_recipients (message, bcc))
    FAIL ();
  if (!smtp_start_session (session))
    SMTPFAIL ("smtp_start_session");

  status = smtp_message_transfer_status (message);
  if (status->code < 200 || status->code > 299) {
    char buf[256];

    snprintf (buf, sizeof (buf), "SMTP error while sending: %d %s",
              status->code, status->text);
    MSGFAIL (buf);
  }

Done:
  if (fp != NULL)
    fclose (fp);
  if (hostportstr != NULL)
    free (hostportstr);
  if (session != NULL)
    smtp_destroy_session (session);
  if (authctx != NULL)
    auth_destroy_context (authctx);

  /* Forget user-entered SMTP AUTH password if send fails */
  if (ret != 0)
    authpass[0] = '\0';

  return ret;
}
