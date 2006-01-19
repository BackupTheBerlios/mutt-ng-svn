/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/pop3_mailbox.cpp
 * @brief Implementation: POP3 Mailbox
 *
 * This file is published under the GNU General Public License.
 */
#include <stdlib.h>

#include "core/str.h"

#include "libmuttng/libmuttng_features.h"
#include "libmuttng/util/url.h"
#include "libmuttng/config/config_manager.h"
#include "libmuttng/config/string_option.h"
#include "pop3_mailbox.h"

/** storage for @ref option_pop_user */
static char* DefaultUser = NULL;
/** storage for @ref option_pop_pass */
static char* DefaultPassword = NULL;

POP3Mailbox::POP3Mailbox (url_t* url_, Connection * c) : RemoteMailbox (url_,c),total(0) {
  this->haveCaching = 1;
  this->haveAuthentication = 1;
  this->haveEncryption = 1;
#if LIBMUTTNG_HAVE_CACHE
  this->cache = Cache::create ();
  connectSignal (this->cache->cacheGetKey, this, &POP3Mailbox::cacheGetKey);
#endif
}

POP3Mailbox::~POP3Mailbox (void) {
#if LIBMUTTNG_HAVE_CACHE
  disconnectSignals (this->cache->cacheGetKey, this);
  delete this->cache;
#endif
}

void POP3Mailbox::reg(void) {
  ConfigManager::regOption(new StringOption("pop_user","",&DefaultUser));
  ConfigManager::regOption(new StringOption("pop_pass","",&DefaultPassword));
  ConfigManager::regFeature("pop3");
}

mailbox_query_status POP3Mailbox::openMailbox() {

  if (conn->ready)
    return MQ_OK;

  if (!url || !conn) {
    /* TODO: emit some error? */
    return MQ_NOT_CONNECTED;
  }

  if (!url->username && !sigGetUsername.emit (url))
    return MQ_AUTH;
  if (!url->password && !sigGetPassword.emit (url))
    return MQ_AUTH;


  if (!url->username || !url->password) {
    return MQ_AUTH;
  }


  if (conn->socketConnect()==false) {
    return MQ_NOT_CONNECTED;
  }

  if (conn->readLine(&rbuf)<=0) {
    return MQ_NOT_CONNECTED;
  }

  if (!buffer_equal1(&rbuf,"+OK",3)) {
    conn->socketDisconnect();
    return MQ_AUTH;
  }

  buffer_add_str(&sbuf,"USER ",5);
  buffer_add_str(&sbuf,url->username,-1);

  if (conn->writeLine(&sbuf)<0) {
    conn->socketDisconnect();
    return MQ_NOT_CONNECTED;
  }

  buffer_shrink(&rbuf,0);

  if (conn->readLine(&rbuf)<=0) {
    return MQ_NOT_CONNECTED;
  }

  if (!buffer_equal1(&rbuf,"+OK",3)) {
    conn->socketDisconnect();
    return MQ_AUTH;
  }

  buffer_shrink(&sbuf,0);
  buffer_add_str(&sbuf,"PASS ",5);
  buffer_add_str(&sbuf,url->password,-1);

  if (conn->writeLine(&sbuf)<0) {
    conn->socketDisconnect();
    return MQ_NOT_CONNECTED;
  }

  buffer_shrink(&rbuf,0);

  if (conn->readLine(&rbuf)<=0) {
    return MQ_NOT_CONNECTED;
  }

  if (!buffer_equal1(&rbuf,"+OK",3)) {
    conn->socketDisconnect();
    return MQ_AUTH;
  }

  /* from this point on, we're successfully logged in and in transactional state */
  conn->ready = true;
  return MQ_OK;
}

bool POP3Mailbox::checkEmpty() {
  /* TODO */
  return true;
}

bool POP3Mailbox::checkACL(acl_bit_t bit) {
  switch (bit) {
  case ACL_INSERT:    /* editing messages */
  case ACL_WRITE:     /* change importance */
    return false;
  case ACL_DELETE:    /* (un)deletion */
  case ACL_SEEN:      /* mark as read */
    return true;
  default:
    return false;
  }
}

mailbox_query_status POP3Mailbox::closeMailbox() {
  /* TODO: synchronize content of mailbox */
  if (conn->ready) {
    buffer_shrink(&sbuf,0);
    buffer_add_str(&sbuf,"RSET",4);
    conn->writeLine(&sbuf);
    conn->readLine(&rbuf);
    buffer_shrink(&sbuf,0);
    buffer_add_str(&sbuf,"QUIT",4);
    conn->writeLine(&sbuf);
    conn->readLine(&rbuf);
    conn->ready = false;
  }
  conn->socketDisconnect();
  return MQ_NOT_CONNECTED;
}

mailbox_query_status POP3Mailbox::syncMailbox() {
  return MQ_NOT_CONNECTED;
}

mailbox_query_status POP3Mailbox::checkMailbox() {
  if (!conn->ready) return MQ_NOT_CONNECTED;

  /* send "GROUP xyz" and... */
  buffer_shrink(&sbuf,0);
  buffer_add_str(&sbuf,"STAT",4);
  if (conn->writeLine(&sbuf)<=0) return MQ_NOT_CONNECTED;
  if (conn->readLine(&rbuf)<=0) return MQ_NOT_CONNECTED;
  /* stop if not okay */
  if (str_ncmp(rbuf.str,"+OK ",4)!=0) return MQ_NOT_CONNECTED;
  /*
    * to avoid scanf() family of functions, we use
    * buffer_extract_token() with an offset into rbuf to
    * space-separate result; to skip initial +OK_ return code,
    * set off to 4 and increase by token length we got while offset
    * still is in rbuf's length; do at most first 1 field
    * (we keep the loop as we may parse mailbox size of STAT later)
    */
  buffer_t tmp;
  buffer_init(&tmp);
  size_t off = 4;
  unsigned short c = 0;
  while ((off += buffer_extract_token2(&tmp,rbuf.str+off,0)) < rbuf.len) {
    switch (++c) {
    case 1: total = atoi(tmp.str); break;
    default: break;
    }
    buffer_shrink(&tmp,0);
  }
  buffer_free(&tmp);

  return MQ_OK;
}

mailbox_query_status POP3Mailbox::commitMessage(Message * msg) {
  (void)msg;
  return MQ_ERR;
}

mailbox_query_status POP3Mailbox::openNewMessage(Message * msg) {
  (void)msg;
  return MQ_ERR;
}

bool POP3Mailbox::checkAccess() {
  return false;
}

mailbox_query_status POP3Mailbox::fetchMessageHeaders(Message * msg, unsigned int msgnum) {
  (void)msg;
  (void)msgnum;

  return MQ_NOT_CONNECTED;
}

mailbox_query_status POP3Mailbox::fetchMessage(Message * msg, unsigned int msgnum) {
  (void)msg;
  (void)msgnum;

  return MQ_NOT_CONNECTED;
}

bool POP3Mailbox::cacheGetKey (Message* msg, buffer_t* dst) {
  (void) msg;
  (void) dst;
  return (false);
}

unsigned long POP3Mailbox::msgNew() { return 0; }
unsigned long POP3Mailbox::msgOld() { return 0; }
unsigned long POP3Mailbox::msgFlagged() { return 0; }
unsigned long POP3Mailbox::msgTotal() { return total; }
