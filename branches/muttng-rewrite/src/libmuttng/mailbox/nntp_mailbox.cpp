/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/nntp_mailbox.cpp
 * @brief Implementation: NNTP Mailbox
 */
#include <stdlib.h>
#include <cstdio>

#include "core/str.h"
#include "core/io.h"

#include "nntp_mailbox.h"
#include "libmuttng/config/config_manager.h"

/** storage for @ref option_nntp_user */
static char* DefaultUser = NULL;
/** storage for @ref option_nntp_pass */
static char* DefaultPassword = NULL;
/** storage for @ref option_nntp_host */
static char* DefaultHost = NULL;
/** locations for which to search for default servers */
static const char* HostFiles[] = {
  "/etc/nntpserver", "/etc/news/server", NULL
};

NNTPMailbox::NNTPMailbox (url_t* url_, Connection * c = NULL) : RemoteMailbox (url_,c) {
  this->haveCaching = 1;
  this->haveAuthentication = 1;
  this->haveEncryption = 1;
  buffer_init(&sbuf);
  buffer_init(&rbuf);
}

NNTPMailbox::~NNTPMailbox (void) {
  buffer_free(&sbuf);
  buffer_free(&rbuf);
}

void NNTPMailbox::reg() {
  char* p;
  if ((p = getenv("NNTPSERVER")) && *p)
    str_replace(&DefaultHost,str_dup(p));
  else {
    int i = 0;
    buffer_t line, host;
    FILE* fp = NULL;
    buffer_init(&line);
    buffer_init(&host);
    for (i = 0; i>=0 && HostFiles[i]; i++) {
      if ((fp = io_fopen(HostFiles[i],"r",-1))) {
        while(io_readline(&line,fp)!=0) {
          buffer_extract_token(&host,&line,0,NULL);
          if (host.str && *host.str != '#') {
            str_replace(&DefaultHost,str_dup(host.str));
            i = -2;
            break;
          }
        }
        io_fclose(&fp);
      }
    }
    buffer_free(&line);
    buffer_free(&host);
  }
  ConfigManager::reg(new StringOption("nntp_user","",&DefaultUser));
  ConfigManager::reg(new StringOption("nntp_pass","",&DefaultPassword));
  ConfigManager::reg(new StringOption("nntp_host",NONULL(DefaultHost),&DefaultHost));
}

bool NNTPMailbox::getSingleCapa(bool* capa, const char* cmd, size_t cmdlen) {
  /* "algorithm": just send command and see if we get 5xx */
  buffer_shrink(&sbuf,0);
  buffer_add_str(&sbuf,cmd,cmdlen);
  *capa = false;
  if (conn->writeLine(&sbuf)<0 || conn->readLine(&rbuf)<0) return false;
  if (str_ncmp(sbuf.str,"500 ",4)!=0)
    *capa = true;
  return true;
}

unsigned long NNTPMailbox::readList(int (*handler)()) {
  bool callback = handler!=NULL;
  unsigned long ret = 0;
  do {
    if (conn->readLine(&rbuf)<0) return 0;
    if (callback && !handler())
      callback = false;
    ret++;
  } while (rbuf.len != 1 && *rbuf.str != '.');
  return ret;
}

bool NNTPMailbox::getAllCapa() {
  if (!getSingleCapa(&haveXOVER,"XOVER",5)) return false;
  if (!getSingleCapa(&haveXPAT,"XPAT",4)) return false;
  if (!getSingleCapa(&haveLISTGROUP,"LISTGROUP",9)) return false;
  if (!getSingleCapa(&haveXGTITLE,"XGTITLE +",9)) return false;
  /* XGTITLE returns a list, i.e. read in but ignore */
  if (str_ncmp(rbuf.str,"282 ",4)==0 && readList()==0) return false;
  /* let's hope group "junk" is defined; XGTITLE should work anyway... */
  if (!getSingleCapa(&haveLISTNEWSGROUPS,"LIST NEWSGROUPS junk",20)) return false;
  if (str_ncmp(rbuf.str,"215 ",4)==0 && readList()==0) return false;
  return true;
}

bool NNTPMailbox::auth() {
  /* emit signals for UI to catch and prompt */
  if (!url->username && !sigGetUsername.emit (url)) return false;
  if (!url->password && !sigGetPassword.emit (url)) return false;
  /* if we have none, we can stop already */
  if (!url->username || !url->password) return false;

  /* send username and ignore result */
  buffer_shrink(&sbuf,0);
  buffer_add_str(&sbuf,"AUTHINFO USER ",14);
  buffer_add_str(&sbuf,url->username,-1);
  if (conn->writeLine(&sbuf)<=0) return false;
  if (conn->readLine(&rbuf)<=0) return false;

  /* send password */
  buffer_shrink(&sbuf,0);
  buffer_add_str(&sbuf,"AUTHINFO PASS ",14);
  buffer_add_str(&sbuf,url->password,-1);
  if (conn->writeLine(&sbuf)<=0) return false;
  if (conn->readLine(&rbuf)<=0) return false;

  /* if response wasn't 281, add response to errorMsg */
  if (str_ncmp(rbuf.str,"281 ",4)!=0) {
    buffer_shrink(&errorMsg,0);
    buffer_add_buffer(&errorMsg,&rbuf);
    buffer_chomp(&errorMsg);
    return false;
  }
  return true;
}

mailbox_query_status NNTPMailbox::quit(mailbox_query_status status) {
  buffer_shrink(&sbuf,0);
  buffer_add_str(&sbuf,"QUIT",4);
  conn->writeLine(&sbuf);
  if (conn->readLine(&rbuf)>0 && errorMsg.len==0 && *rbuf.str!='2')
    /* just to make sure we have some message */
    buffer_add_buffer(&errorMsg,&rbuf);
  return status;
}

mailbox_query_status NNTPMailbox::openMailbox() {

  if (!url || !conn) {
    /* TODO: emit some error? */
    return MQ_NOT_CONNECTED;
  }

  buffer_shrink(&errorMsg,0);

  if (conn->socketConnect()==false) return MQ_NOT_CONNECTED;
  if (conn->readLine(&rbuf)<=0 && *rbuf.str!='2') return quit(MQ_NOT_CONNECTED);

  /* MODE READER already is an extension of RfC2980 but... */
  buffer_add_str(&sbuf,"MODE READER",11);
  if (conn->writeLine(&sbuf)<=0) return MQ_NOT_CONNECTED;
  if (conn->readLine(&rbuf)<=0) return MQ_NOT_CONNECTED;
  /* MODE READER returned 480 and authentication failed? */
  if (str_ncmp(rbuf.str,"480 ",4)==0 && !auth()) quit(MQ_AUTH);

  /* attempt to get supported extensions */
  if (!getAllCapa()) return MQ_NOT_CONNECTED;

  DEBUGPRINT(D_PARSE,("%s(xover|xpat|listgroup|xgtitle|list newsgroups): "
                      "%d|%d|%d|%d|%d",url->host,haveXOVER,haveXPAT,haveLISTGROUP,
                      haveXGTITLE,haveLISTNEWSGROUPS));

  /* we're connected */
  return MQ_OK;
}

bool NNTPMailbox::checkEmpty() {
  return true;
}

bool NNTPMailbox::checkACL(acl_bit_t bit) {
  switch (bit) {
  case ACL_DELETE:
  case ACL_SEEN:
    return true;
  case ACL_INSERT:
  case ACL_WRITE:
  default:
    return false;
  }
}

mailbox_query_status NNTPMailbox::closeMailbox() {
  mailbox_query_status ret = quit(MQ_NOT_CONNECTED);
  conn->socketDisconnect();
  return ret;
}

mailbox_query_status NNTPMailbox::syncMailbox() {
  return MQ_NOT_CONNECTED;
}

mailbox_query_status NNTPMailbox::checkMailbox() {
  return MQ_NOT_CONNECTED;
}

mailbox_query_status NNTPMailbox::commitMessage(Message * msg) {
  (void)msg;
  return MQ_ERR;
}

mailbox_query_status NNTPMailbox::openNewMessage(Message * msg) {
  (void)msg;
  return MQ_ERR;
}

bool NNTPMailbox::checkAccess() {
  return false;
}

mailbox_query_status NNTPMailbox::fetchMessageHeaders(Message * msg, unsigned int msgnum) {
  (void)msg;
  (void)msgnum;
  
  return MQ_NOT_CONNECTED;
}

mailbox_query_status NNTPMailbox::fetchMessage(Message * msg, unsigned int msgnum) {
  (void)msg;
  (void)msgnum;
  
  return MQ_NOT_CONNECTED;
}

bool NNTPMailbox::cacheGetKey (Message* msg, buffer_t* dst) {
  (void) msg;
  (void) dst;
  return (false);
}
