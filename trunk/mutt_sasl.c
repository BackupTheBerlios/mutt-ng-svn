/*
 * Copyright notice from original mutt:
 * Copyright (C) 2000-5 Brendan Cully <brendan@kublai.com>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

/* common SASL helper routines */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "mutt.h"
#include "account.h"
#include "mutt_sasl.h"
#include "mutt_socket.h"

#include "lib/mem.h"
#include "lib/debug.h"

#include <errno.h>
#include <netdb.h>
#include <sasl/sasl.h>

#include <sys/socket.h>
#include <netinet/in.h>

static int getnameinfo_err (int ret)
{
  int err;

  switch (ret) {
  case EAI_AGAIN:
    debug_print (1, ("The name could not be resolved at this time. Future attempts may succeed.\n"));
    err = SASL_TRYAGAIN;
    break;
  case EAI_BADFLAGS:
    debug_print (1, ("The flags had an invalid value.\n"));
    err = SASL_BADPARAM;
    break;
  case EAI_FAIL:
    debug_print (1, ("A non-recoverable error occurred.\n"));
    err = SASL_FAIL;
    break;
  case EAI_FAMILY:
    debug_print (1, ("The address family was not recognized or the address length was invalid for the specified family.\n"));
    err = SASL_BADPROT;
    break;
  case EAI_MEMORY:
    debug_print (1, ("There was a memory allocation failure.\n"));
    err = SASL_NOMEM;
    break;
  case EAI_NONAME:
    debug_print (1, ("The name does not resolve for the supplied parameters. NI_NAMEREQD is set and the host's name cannot be located, or both nodename and servname were null.\n"));
    err = SASL_FAIL;            /* no real equivalent */
    break;
  case EAI_SYSTEM:
    debug_print (1, ("A system error occurred.  The error code can be found in errno(%d,%s)).\n", 
                errno, strerror (errno)));
    err = SASL_FAIL;            /* no real equivalent */
    break;
  default:
    debug_print (1, ("Unknown error %d\n", ret));
    err = SASL_FAIL;            /* no real equivalent */
    break;
  }
  return err;
}

/* arbitrary. SASL will probably use a smaller buffer anyway. OTOH it's
 * been a while since I've had access to an SASL server which negotiated
 * a protection buffer. */
#define M_SASL_MAXBUF 65536

#define IP_PORT_BUFLEN 1024

static sasl_callback_t mutt_sasl_callbacks[5];

static int mutt_sasl_start (void);

/* callbacks */
static int mutt_sasl_cb_log (void *context, int priority,
                             const char *message);
static int mutt_sasl_cb_authname (void *context, int id, const char **result,
                                  unsigned int *len);
static int mutt_sasl_cb_pass (sasl_conn_t * conn, void *context, int id,
                              sasl_secret_t ** psecret);

/* socket wrappers for a SASL security layer */
static int mutt_sasl_conn_open (CONNECTION * conn);
static int mutt_sasl_conn_close (CONNECTION * conn);
static int mutt_sasl_conn_read (CONNECTION * conn, char *buf, size_t len);
static int mutt_sasl_conn_write (CONNECTION * conn, const char *buf,
                                 size_t count);

/* utility function, stolen from sasl2 sample code */
static int iptostring (const struct sockaddr *addr, socklen_t addrlen,
                       char *out, unsigned outlen)
{
  char hbuf[NI_MAXHOST], pbuf[NI_MAXSERV];
  int ret;

  if (!addr || !out)
    return SASL_BADPARAM;

  ret = getnameinfo (addr, addrlen, hbuf, sizeof (hbuf), pbuf, sizeof (pbuf),
                     NI_NUMERICHOST |
#ifdef NI_WITHSCOPEID
                     NI_WITHSCOPEID |
#endif
                     NI_NUMERICSERV);
  if (ret)
    return getnameinfo_err (ret);

  if (outlen < str_len (hbuf) + str_len (pbuf) + 2)
    return SASL_BUFOVER;

  snprintf (out, outlen, "%s;%s", hbuf, pbuf);

  return SASL_OK;
}

/* mutt_sasl_start: called before doing a SASL exchange - initialises library
 *   (if necessary). */
int mutt_sasl_start (void)
{
  static unsigned char sasl_init = 0;

  static sasl_callback_t callbacks[2];
  int rc;

  if (sasl_init)
    return SASL_OK;

  /* set up default logging callback */
  callbacks[0].id = SASL_CB_LOG;
  callbacks[0].proc = mutt_sasl_cb_log;
  callbacks[0].context = NULL;

  callbacks[1].id = SASL_CB_LIST_END;
  callbacks[1].proc = NULL;
  callbacks[1].context = NULL;

  rc = sasl_client_init (callbacks);

  if (rc != SASL_OK) {
    debug_print (1, ("libsasl initialisation failed.\n"));
    return SASL_FAIL;
  }

  sasl_init = 1;

  return SASL_OK;
}

/* mutt_sasl_client_new: wrapper for sasl_client_new which also sets various
 * security properties. If this turns out to be fine for POP too we can
 * probably stop exporting mutt_sasl_get_callbacks(). */
int mutt_sasl_client_new (CONNECTION * conn, sasl_conn_t ** saslconn)
{
  sasl_security_properties_t secprops;

  struct sockaddr_storage local, remote;
  socklen_t size;
  char iplocalport[IP_PORT_BUFLEN], ipremoteport[IP_PORT_BUFLEN];
  const char *service;
  int rc;

  if (mutt_sasl_start () != SASL_OK)
    return -1;

  switch (conn->account.type) {
  case M_ACCT_TYPE_IMAP:
    service = "imap";
    break;
  case M_ACCT_TYPE_POP:
    service = "pop";
    break;
  default:
    debug_print (1, ("account type unset\n"));
    return -1;
  }

  size = sizeof (local);
  if (getsockname (conn->fd, (struct sockaddr *) &local, &size)) {
    debug_print (1, ("getsockname for local failed\n"));
    return -1;
  }
  else
    if (iptostring
        ((struct sockaddr *) &local, size, iplocalport,
         IP_PORT_BUFLEN) != SASL_OK) {
    debug_print (1, ("iptostring for local failed\n"));
    return -1;
  }

  size = sizeof (remote);
  if (getpeername (conn->fd, (struct sockaddr *) &remote, &size)) {
    debug_print (1, ("getsockname for remote failed\n"));
    return -1;
  }
  else
    if (iptostring
        ((struct sockaddr *) &remote, size, ipremoteport,
         IP_PORT_BUFLEN) != SASL_OK) {
    debug_print (1, ("iptostring for remote failed\n"));
    return -1;
  }

  debug_print (1, ("local ip: %s, remote ip:%s\n", iplocalport, ipremoteport));

  rc =
    sasl_client_new (service, conn->account.host, iplocalport, ipremoteport,
                     mutt_sasl_get_callbacks (&conn->account), 0, saslconn);

  if (rc != SASL_OK) {
    debug_print (1, ("Error allocating SASL connection\n"));
    return -1;
  }

  /*** set sasl IP properties, necessary for use with krb4 ***/
  /* Do we need to fail if this fails? I would assume having these unset
   * would just disable KRB4. Who wrote this code?
   */
  {
    struct sockaddr_in local, remote;
    socklen_t size;

    size = sizeof (local);
    if (getsockname (conn->fd, (struct sockaddr *) &local, &size))
      return -1;

    size = sizeof (remote);
    if (getpeername (conn->fd, (struct sockaddr *) &remote, &size))
      return -1;

#ifdef SASL_IP_LOCAL
    if (sasl_setprop (*saslconn, SASL_IP_LOCAL, &local) != SASL_OK) {
      debug_print (1, ("Error setting local IP address\n"));
      return -1;
    }
#endif

#ifdef SASL_IP_REMOTE
    if (sasl_setprop (*saslconn, SASL_IP_REMOTE, &remote) != SASL_OK) {
      debug_print (1, ("Error setting remote IP address\n"));
      return -1;
    }
#endif
  }

  /* set security properties. We use NOPLAINTEXT globally, since we can
   * just fall back to LOGIN in the IMAP case anyway. If that doesn't
   * work for POP, we can make it a flag or move this code into
   * imap/auth_sasl.c */
  memset (&secprops, 0, sizeof (secprops));
  /* Work around a casting bug in the SASL krb4 module */
  secprops.max_ssf = 0x7fff;
  secprops.maxbufsize = M_SASL_MAXBUF;
  secprops.security_flags |= SASL_SEC_NOPLAINTEXT;
  if (sasl_setprop (*saslconn, SASL_SEC_PROPS, &secprops) != SASL_OK) {
    debug_print (1, ("Error setting security properties\n"));
    return -1;
  }

  if (conn->ssf) {
    debug_print (2, ("External SSF: %d\n", conn->ssf));
    if (sasl_setprop (*saslconn, SASL_SSF_EXTERNAL, &(conn->ssf)) != SASL_OK)
    {
      debug_print (1, ("Error setting external properties\n"));
      return -1;
    }
    debug_print (2, ("External authentication name: %s\n", conn->account.user));
    if (sasl_setprop (*saslconn, SASL_AUTH_EXTERNAL, conn->account.user) !=
        SASL_OK) {
      debug_print (1, ("Error setting external properties\n"));
      return -1;
    }
  }

  return 0;
}

sasl_callback_t *mutt_sasl_get_callbacks (ACCOUNT * account)
{
  sasl_callback_t *callback;

  callback = mutt_sasl_callbacks;

  callback->id = SASL_CB_USER;
  callback->proc = mutt_sasl_cb_authname;
  callback->context = account;
  callback++;

  callback->id = SASL_CB_AUTHNAME;
  callback->proc = mutt_sasl_cb_authname;
  callback->context = account;
  callback++;

  callback->id = SASL_CB_PASS;
  callback->proc = mutt_sasl_cb_pass;
  callback->context = account;
  callback++;

  callback->id = SASL_CB_GETREALM;
  callback->proc = NULL;
  callback->context = NULL;
  callback++;

  callback->id = SASL_CB_LIST_END;
  callback->proc = NULL;
  callback->context = NULL;

  return mutt_sasl_callbacks;
}

int mutt_sasl_interact (sasl_interact_t * interaction)
{
  char prompt[SHORT_STRING];
  char resp[SHORT_STRING];

  while (interaction->id != SASL_CB_LIST_END) {
    debug_print (2, ("filling in SASL interaction %ld.\n", interaction->id));

    snprintf (prompt, sizeof (prompt), "%s: ", interaction->prompt);
    resp[0] = '\0';
    if (mutt_get_field (prompt, resp, sizeof (resp), 0))
      return SASL_FAIL;

    interaction->len = str_len (resp) + 1;
    interaction->result = mem_malloc (interaction->len);
    memcpy ((char*) interaction->result, resp, interaction->len);

    interaction++;
  }

  return SASL_OK;
}

/* SASL can stack a protection layer on top of an existing connection.
 * To handle this, we store a saslconn_t in conn->sockdata, and write
 * wrappers which en/decode the read/write stream, then replace sockdata
 * with an embedded copy of the old sockdata and call the underlying
 * functions (which we've also preserved). I thought about trying to make
 * a general stackable connection system, but it seemed like overkill -
 * something is wrong if we have 15 filters on top of a socket. Anyway,
 * anything else which wishes to stack can use the same method. The only
 * disadvantage is we have to write wrappers for all the socket methods,
 * even if we only stack over read and write. Thinking about it, the
 * abstraction problem is that there is more in CONNECTION than there
 * needs to be. Ideally it would have only (void*)data and methods. */

/* mutt_sasl_setup_conn: replace connection methods, sockdata with 
 *   SASL wrappers, for protection layers. Also get ssf, as a fastpath
 *   for the read/write methods. */
void mutt_sasl_setup_conn (CONNECTION * conn, sasl_conn_t * saslconn)
{
  SASL_DATA *sasldata = (SASL_DATA *) mem_malloc (sizeof (SASL_DATA));

  sasldata->saslconn = saslconn;
  /* get ssf so we know whether we have to (en|de)code read/write */
  sasl_getprop (saslconn, SASL_SSF, (const void **) &sasldata->ssf);
  debug_print (3, ("SASL protection strength: %u\n", *sasldata->ssf));
  /* Add SASL SSF to transport SSF */
  conn->ssf += *sasldata->ssf;
  sasl_getprop (saslconn, SASL_MAXOUTBUF,
                (const void **) &sasldata->pbufsize);
  debug_print (3, ("SASL protection buffer size: %u\n", *sasldata->pbufsize));

  /* clear input buffer */
  sasldata->buf = NULL;
  sasldata->bpos = 0;
  sasldata->blen = 0;

  /* preserve old functions */
  sasldata->sockdata = conn->sockdata;
  sasldata->msasl_open = conn->conn_open;
  sasldata->msasl_close = conn->conn_close;
  sasldata->msasl_read = conn->conn_read;
  sasldata->msasl_write = conn->conn_write;

  /* and set up new functions */
  conn->sockdata = sasldata;
  conn->conn_open = mutt_sasl_conn_open;
  conn->conn_close = mutt_sasl_conn_close;
  conn->conn_read = mutt_sasl_conn_read;
  conn->conn_write = mutt_sasl_conn_write;
}

void mutt_sasl_done (void) {
  sasl_done ();
}

/* mutt_sasl_cb_log: callback to log SASL messages */
static int mutt_sasl_cb_log (void *context, int priority, const char *message)
{
  debug_print (priority, ("SASL: %s\n", message));

  return SASL_OK;
}

/* mutt_sasl_cb_authname: callback to retrieve authname or user from ACCOUNT */
static int mutt_sasl_cb_authname (void *context, int id, const char **result,
                                  unsigned *len)
{
  ACCOUNT *account = (ACCOUNT *) context;

  *result = NULL;
  if (len)
    *len = 0;

  if (!account)
    return SASL_BADPARAM;

  debug_print (2, ("getting %s for %s:%u\n",
              id == SASL_CB_AUTHNAME ? "authname" : "user",
              account->host, account->port));

  if (id == SASL_CB_AUTHNAME) {
    if (mutt_account_getlogin (account))
      return SASL_FAIL;
    *result = account->login;
  } else {
    if (mutt_account_getuser (account))
      return SASL_FAIL;
    *result = account->user;
  }

  if (len)
    *len = str_len (*result);

  return SASL_OK;
}

static int mutt_sasl_cb_pass (sasl_conn_t * conn, void *context, int id,
                              sasl_secret_t ** psecret)
{
  ACCOUNT *account = (ACCOUNT *) context;
  int len;

  if (!account || !psecret)
    return SASL_BADPARAM;

  debug_print (2, ("getting password for %s@%s:%u\n",
              account->login, account->host, account->port));

  if (mutt_account_getpass (account))
    return SASL_FAIL;

  len = str_len (account->pass);

  *psecret = (sasl_secret_t *) mem_malloc (sizeof (sasl_secret_t) + len);
  (*psecret)->len = len;
  strcpy ((char*) (*psecret)->data, account->pass);     /* __STRCPY_CHECKED__ */

  return SASL_OK;
}

/* mutt_sasl_conn_open: empty wrapper for underlying open function. We
 *   don't know in advance that a connection will use SASL, so we
 *   replace conn's methods with sasl methods when authentication
 *   is successful, using mutt_sasl_setup_conn */
static int mutt_sasl_conn_open (CONNECTION * conn)
{
  SASL_DATA *sasldata;
  int rc;

  sasldata = (SASL_DATA *) conn->sockdata;
  conn->sockdata = sasldata->sockdata;
  rc = (sasldata->msasl_open) (conn);
  conn->sockdata = sasldata;

  return rc;
}

/* mutt_sasl_conn_close: calls underlying close function and disposes of
 *   the sasl_conn_t object, then restores connection to pre-sasl state */
static int mutt_sasl_conn_close (CONNECTION * conn)
{
  SASL_DATA *sasldata;
  int rc;

  sasldata = (SASL_DATA *) conn->sockdata;

  /* restore connection's underlying methods */
  conn->sockdata = sasldata->sockdata;
  conn->conn_open = sasldata->msasl_open;
  conn->conn_close = sasldata->msasl_close;
  conn->conn_read = sasldata->msasl_read;
  conn->conn_write = sasldata->msasl_write;

  /* release sasl resources */
  sasl_dispose (&sasldata->saslconn);
  mem_free (&sasldata->buf);
  mem_free (&sasldata);

  /* call underlying close */
  rc = (conn->conn_close) (conn);

  return rc;
}

static int mutt_sasl_conn_read (CONNECTION * conn, char *buf, size_t len)
{
  SASL_DATA *sasldata;
  int rc;

  unsigned int olen;

  sasldata = (SASL_DATA *) conn->sockdata;

  /* if we still have data in our read buffer, copy it into buf */
  if (sasldata->blen > sasldata->bpos) {
    olen = (sasldata->blen - sasldata->bpos > len) ? len :
      sasldata->blen - sasldata->bpos;

    memcpy (buf, sasldata->buf + sasldata->bpos, olen);
    sasldata->bpos += olen;

    return olen;
  }

  conn->sockdata = sasldata->sockdata;

  mem_free (&sasldata->buf);
  sasldata->bpos = 0;
  sasldata->blen = 0;

  /* and decode the result, if necessary */
  if (*sasldata->ssf) {
    do {
      /* call the underlying read function to fill the buffer */
      rc = (sasldata->msasl_read) (conn, buf, len);
      if (rc <= 0)
        goto out;

      rc = sasl_decode (sasldata->saslconn, buf, rc, &sasldata->buf,
                        &sasldata->blen);
      if (rc != SASL_OK) {
        debug_print (1, ("SASL decode failed: %s\n",
                    sasl_errstring (rc, NULL, NULL)));
        goto out;
      }
    }
    while (!sasldata->blen);

    olen = (sasldata->blen - sasldata->bpos > len) ? len :
      sasldata->blen - sasldata->bpos;

    memcpy (buf, sasldata->buf, olen);
    sasldata->bpos += olen;

    rc = olen;
  }
  else
    rc = (sasldata->msasl_read) (conn, buf, len);

out:
  conn->sockdata = sasldata;

  return rc;
}

static int mutt_sasl_conn_write (CONNECTION * conn, const char *buf,
                                 size_t len)
{
  SASL_DATA *sasldata;
  int rc;

  const char *pbuf;
  unsigned int olen, plen;

  sasldata = (SASL_DATA *) conn->sockdata;
  conn->sockdata = sasldata->sockdata;

  /* encode data, if necessary */
  if (*sasldata->ssf) {
    /* handle data larger than MAXOUTBUF */
    do {
      olen = (len > *sasldata->pbufsize) ? *sasldata->pbufsize : len;

      rc = sasl_encode (sasldata->saslconn, buf, olen, &pbuf, &plen);
      if (rc != SASL_OK) {
        debug_print (1, ("SASL encoding failed: %s\n",
                    sasl_errstring (rc, NULL, NULL)));
        goto fail;
      }

      rc = (sasldata->msasl_write) (conn, pbuf, plen);
      mem_free (&pbuf);
      if (rc != plen)
        goto fail;

      len -= olen;
      buf += olen;
    }
    while (len > *sasldata->pbufsize);
  }
  else
    /* just write using the underlying socket function */
    rc = (sasldata->msasl_write) (conn, buf, len);

  conn->sockdata = sasldata;

  return rc;

fail:
  conn->sockdata = sasldata;
  return -1;
}
