/*
 * Copyright notice from original mutt:
 * [none]
 *
 * Parts were written/modified by:
 * Christian Gall <cg@cgall.de>
 * Rocco Rutte <pdmef@cs.tu-berlin.de>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */
#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "mutt.h"
#include "ascii.h"
#include "enter.h"

#include "lib/mem.h"
#include "lib/intl.h"
#include "lib/str.h"

#if defined (USE_SSL) || (defined (USE_GNUTLS) && defined (HAVE_GNUTLS_OPENSSL_H))
#include <openssl/ssl.h>
#endif

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
#define extna(msg) { mutt_error (_("SMTP Extension '%s' not supported by MTA."), \
                                 msg); sleep (1); }

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
          mutt_get_field_unbuffered (prompt, authpass, sizeof (authpass),
                                     M_PASS);
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

static int handle_invalid_peer_certificate (long vfy_result) {
#if defined (USE_SSL) || (defined (USE_GNUTLS) && defined (HAVE_GNUTLS_OPENSSL_H))
  mutt_error (_("Error verifying certificate: %s"),
              NONULL (X509_verify_cert_error_string (vfy_result)));
#else
  mutt_error (_("Error verifying certificate. Error Code: %lu"), vfy_result);
#endif
  sleep(2);
  return 1; /* Accept the problem */
}

static void event_cb (smtp_session_t session, int event_no, void *arg,...)
{ 
  va_list alist;
  int *ok;

  va_start(alist, arg);
  switch(event_no) {
  case SMTP_EV_CONNECT:
  case SMTP_EV_MAILSTATUS:
  case SMTP_EV_RCPTSTATUS:
  case SMTP_EV_MESSAGEDATA:
  case SMTP_EV_MESSAGESENT:
  case SMTP_EV_DISCONNECT: break;
  case SMTP_EV_WEAK_CIPHER: {
    int bits;
    bits = va_arg(alist, long); ok = va_arg(alist, int*);
    mutt_message (_("SMTP_EV_WEAK_CIPHER, bits=%d - accepted."), bits);
    sleep(1);
    *ok = 1; break;
  } 
  case SMTP_EV_STARTTLS_OK:
    mutt_message (_("Using TLS"));
    sleep(1);
    break;
  case SMTP_EV_INVALID_PEER_CERTIFICATE: {
    long vfy_result;
    vfy_result = va_arg(alist, long); ok = va_arg(alist, int*);
    *ok = handle_invalid_peer_certificate(vfy_result);
    sleep(1);
    break;
  } 
  case SMTP_EV_NO_PEER_CERTIFICATE: {
    ok = va_arg(alist, int*); 
    mutt_message (_("SMTP_EV_NO_PEER_CERTIFICATE - accepted."));
    sleep(1);
    *ok = 1; break;
  }
  case SMTP_EV_WRONG_PEER_CERTIFICATE: {
    ok = va_arg(alist, int*);
    mutt_message (_("SMTP_EV_WRONG_PEER_CERTIFICATE - accepted."));
    sleep(1);
    *ok = 1; break;
  }
  case SMTP_EV_NO_CLIENT_CERTIFICATE: {
    ok = va_arg(alist, int*);
    mutt_message (_("SMTP_EV_NO_CLIENT_CERTIFICATE - accepted."));
    sleep(1);
    *ok = 1; break;
  }
  case SMTP_EV_EXTNA_DSN:
    extna ("DSN");
    break;
  case SMTP_EV_EXTNA_STARTTLS:
    extna ("StartTLS");
    break;
  case SMTP_EV_EXTNA_8BITMIME:
    extna ("8BITMIME");
    break;
  default:
    mutt_message(_("Got unhandled event ID = %d - ignored."), event_no);
    sleep(1);
  }
  va_end(alist);
}

static void do_dsn_notify (smtp_message_t message, const char* from) {
  int flags = Notify_NOTSET;
  smtp_recipient_t self = NULL;

  if (!DsnNotify || !*DsnNotify || !message || !from || !*from || 
      strstr (DsnNotify, "never") != NULL)
    return;

  if (strstr (DsnNotify, "failure") != NULL)
    flags |= Notify_FAILURE;
  if (strstr (DsnNotify, "delay") != NULL)
    flags |= Notify_DELAY;
  if (strstr (DsnNotify, "success") != NULL)
    flags |= Notify_SUCCESS;

  if (flags != Notify_NOTSET) {
    if (!(self = smtp_add_recipient (message, from)))
      return;
    smtp_dsn_set_notify (self, flags);
  }
}

static void do_dsn_ret (smtp_message_t message) {
  if (!DsnReturn || !*DsnReturn || !message)
    return;
  if (ascii_strncasecmp (DsnReturn, "hdrs", 4) == 0)
    smtp_dsn_set_ret (message, Ret_HDRS);
  else if (ascii_strncasecmp (DsnReturn, "full", 4) == 0)
    smtp_dsn_set_ret (message, Ret_FULL);
}

#if defined (USE_LIBESMTP) && (defined (USE_SSL) || defined (USE_GNUTLS))
int mutt_libesmtp_check_usetls (const char* option, unsigned long p,
                                char* errbuf, size_t errlen) {
  char* val = (char*) p;
  if (!val || !*val)
    return (1);
  if (str_ncmp (val, "enabled", 7) != 0 &&
      str_ncmp (val, "required", 8) != 0) {
    if (errbuf)
      snprintf (errbuf, errlen, _("'%s' is invalid for %s"), val, option);
    return (0);
  }
  return (1);
}
#endif

/*
 * mutt_libesmtp_invoke
 *   Sends a mail message to the provided recipients using libesmtp.
 *   Returns 0 upon success, -1 upon failure (and prints an error
 *   message).
 */
int mutt_libesmtp_invoke (ADDRESS * from,       /* the sender */
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
  char* envfrom = from->mailbox;

  _mutt_libesmtp_ensure_init ();

  if ((session = smtp_create_session ()) == NULL)
    SMTPFAIL ("smtp_create_session");

#if defined (USE_SSL) || (defined (USE_GNUTLS) && defined (HAVE_GNUTLS_OPENSSL_H))
  if (SmtpUseTLS != NULL && ascii_strncasecmp("enabled", SmtpUseTLS, 7) == 0) {
    smtp_starttls_enable(session, Starttls_ENABLED);
  } else if (SmtpUseTLS != NULL && ascii_strncasecmp("required", SmtpUseTLS, 8) == 0) {
    smtp_starttls_enable(session, Starttls_REQUIRED);
  }
#endif

  /* Create hostname:port string and tell libesmtp */
  /* len = SmtpHost len + colon + max port (65536 => 5 chars) + terminator */
  hostportlen = str_len (SmtpHost) + 7;
  hostportstr = mem_malloc (hostportlen);
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

#if defined (USE_SSL) || (defined (USE_GNUTLS) && defined (HAVE_GNUTLS_OPENSSL_H))
  smtp_starttls_set_ctx (session, NULL);
#endif
  smtp_set_eventcb (session, event_cb, NULL);

  if ((message = smtp_add_message (session)) == NULL)
    SMTPFAIL ("smtp_add_message");

  /*  Initialize envelope sender */
  if (option (OPTENVFROM) && EnvFrom)
    envfrom = EnvFrom->mailbox;
  if (!smtp_set_reverse_path (message, envfrom))
    SMTPFAIL ("smtp_set_reverse_path");

  /* set up DSN for message */
  do_dsn_notify (message, envfrom);
  do_dsn_ret (message);

  /* set up 8bitmime flag */
  if (eightbit && option (OPTUSE8BITMIME))
    smtp_8bitmime_set_body (message, E8bitmime_8BITMIME);

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
    mem_free (&hostportstr);
  if (session != NULL)
    smtp_destroy_session (session);
  if (authctx != NULL)
    auth_destroy_context (authctx);

  /* Forget user-entered SMTP AUTH password if send fails */
  if (ret != 0)
    authpass[0] = '\0';

  return ret;
}
