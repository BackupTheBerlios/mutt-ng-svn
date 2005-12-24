/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/nntp_mailbox.cpp
 * @brief Implementation: NNTP Mailbox
 */
#include <stdlib.h>
#include <vector>
#include <cstdio>

#include "core/str.h"
#include "core/io.h"
#include "core/intl.h"

#include "nntp_mailbox.h"
#include "libmuttng/config/config_manager.h"

/**
 * Fetch context: server's article numbers
 */
typedef struct ctx_t {
  /** first */
  unsigned long first;
  /** last */
  unsigned long last;
  /** init */
  ctx_t() { first = 0; last = 0; }
} ctx_t;

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

/**
 * Callback for readList(): parses result of LISTGROUP command
 * @param line NNTPMailbox::rbuf.
 * @param data Pointer to ctx_t casted to void*.
 * @return true.
 */
static bool handleListgroup(buffer_t* line, void* data) {
  ctx_t* ctx = (ctx_t*) data;
  if (ctx->first == 0)
    ctx->first = atoi(line->str);
  ctx->last = atoi(line->str);
  return true;
}

NNTPMailbox::NNTPMailbox (url_t* url_, Connection * c = NULL) : RemoteMailbox (url_,c), first(0),last(0),total(0) {
  this->haveCaching = 1;
  this->haveAuthentication = 1;
  this->haveEncryption = 1;
  buffer_init(&errorMsg);
}

NNTPMailbox::~NNTPMailbox (void) {
  buffer_free(&errorMsg);
}

void NNTPMailbox::reg() {
  char* p;
  if ((p = getenv("NNTPSERVER")) && *p)
    str_replace(&DefaultHost,p);
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
            str_replace(&DefaultHost,host.str);
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
  if (conn->writeLine(&sbuf)<=0 || conn->readLine(&rbuf)<=0) return false;
  if (str_ncmp(sbuf.str,"500 ",4)!=0)
    *capa = true;
  return true;
}

unsigned long NNTPMailbox::readList(bool (*handler)(buffer_t* line, void* data), void* data) {
  bool callback = handler!=NULL;
  unsigned long ret = 0;
  if (conn->readLine(&rbuf)<0) return 0;
  while (rbuf.len != 1 && *rbuf.str != '.') {
    if (callback && !handler(&rbuf,data))
      callback = false;
    ret++;
    if (conn->readLine(&rbuf)<0) return 0;
  }
  return ++ret;
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
    makeError();
    return false;
  }
  return true;
}

mailbox_query_status NNTPMailbox::quit(mailbox_query_status status) {
  buffer_shrink(&sbuf,0);
  buffer_add_str(&sbuf,"QUIT",4);
  conn->writeLine(&sbuf);
  if (conn->readLine(&rbuf)>0 && errorMsg.len==0 && *rbuf.str!='2')
    makeError();
  conn->ready = false;
  return status;
}

mailbox_query_status NNTPMailbox::openMailbox() {

  if (!url || !conn) {
    /* TODO: emit some error? */
    return MQ_NOT_CONNECTED;
  }

  if (conn->ready)
    return MQ_OK;

  buffer_shrink(&errorMsg,0);

  if (!*(url->path+1)) {
    /*
     * do it in two steps so we have only 1 translated message in case
     * we use the first part elsewhere, too
     */
    buffer_add_str(&errorMsg,_("Invalid newsgroup '"),-1);
    buffer_add_str(&errorMsg,_("'"),-1);
    displayError->emit(&errorMsg);
    return MQ_NOT_CONNECTED;
  }

  if (conn->socketConnect()==false) {
    return MQ_NOT_CONNECTED;
  }

  if (conn->readLine(&rbuf)<=0) return MQ_NOT_CONNECTED;
  if (*rbuf.str!='2') {
    makeError();
    return quit(MQ_NOT_CONNECTED);
  }

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
  conn->ready = true;
  return MQ_OK;
}

bool NNTPMailbox::checkEmpty() {
  return groupStat() && total != 0;
}

bool NNTPMailbox::checkACL(acl_bit_t bit) {
  switch (bit) {
  case ACL_DELETE:
  case ACL_SEEN:
    return true;
  default:
    return false;
  }
}

mailbox_query_status NNTPMailbox::closeMailbox() {
  return MQ_NOT_CONNECTED;
}

mailbox_query_status NNTPMailbox::syncMailbox() {
  return MQ_NOT_CONNECTED;
}

void NNTPMailbox::makeError() {
  buffer_shrink(&errorMsg,0);
  if (rbuf.len) {
    buffer_add_str(&errorMsg,_("Server responded: '"),-1);
    buffer_add_buffer(&errorMsg,&rbuf);
    buffer_add_str(&errorMsg,_("'"),-1);
    buffer_chomp(&errorMsg);
  }
  DEBUGPRINT(D_MOD,("%s",errorMsg.str));
  displayError->emit(&errorMsg);
  conn->ready = false;
}

bool NNTPMailbox::groupStat() {
  buffer_shrink(&sbuf,0);
  first = 0;
  last = 0;
  total = 0;

  /**
   * LISTGROUP makes server print all articles as lists; as we don't have
   * caching yet, we don't need to know about the exact numbers yet so
   * we temporarily use GROUP command (maybe this remain as we can deal
   * with stale articles in other ways; LISTGROUPS also increases
   * traffic quite a lot... XXX
   */
  haveLISTGROUP = false;

  if (haveLISTGROUP) {
    ctx_t ctx;
    /* send "LISTGROUP xyz" and... */
    buffer_add_str(&sbuf,"LISTGROUP ",10);
    buffer_add_str(&sbuf,url->path+1,-1);
    if (conn->writeLine(&sbuf)<=0) return false;
    if (conn->readLine(&rbuf)<=0) return false;
    /* stop if we don't get 211 */
    if (str_ncmp(rbuf.str,"211 ",4)!=0) {
      makeError();
      return false;
    }
    /* count */
    if ((total = readList(&handleListgroup,(void*)&ctx)-1)<=0) return false;
    first = ctx.first;
    last = ctx.last;
  } else {
    /* send "GROUP xyz" and... */
    buffer_add_str(&sbuf,"GROUP ",6);
    buffer_add_str(&sbuf,url->path+1,-1);
    if (conn->writeLine(&sbuf)<=0) return false;
    if (conn->readLine(&rbuf)<=0) return false;
    /* stop if we don't get 211 */
    if (str_ncmp(rbuf.str,"211 ",4)!=0) {
      makeError();
      return false;
    }
    /*
     * to avoid scanf() family of functions, we use
     * buffer_extract_token() with an offset into rbuf to
     * space-separate result; to skip initial 211 return code,
     * set off to 4 and increase by token length we got while offset
     * still is in rbuf's length; do at most first 3 fields
     */
    buffer_t tmp;
    buffer_init(&tmp);
    size_t off = 4;
    unsigned short c = 0;
    while ((off += buffer_extract_token2(&tmp,rbuf.str+off,0,NULL)) < rbuf.len) {
      switch (++c) {
      case 1: total = atoi(tmp.str); break;
      case 2: first = atoi(tmp.str); break;
      case 3: last = atoi(tmp.str); break;
      default: break;
      }
      buffer_shrink(&tmp,0);
    }
    buffer_free(&tmp);
  }

  DEBUGPRINT(D_PARSE,("group %s: %d:%d==%d",url->path+1,first,last,total));

  return true;
}

mailbox_query_status NNTPMailbox::checkMailbox() {
  mailbox_query_status state = MQ_OK;

  /* if not opened yet, try it */
  if (!conn->ready && (state = openMailbox())!=MQ_OK)
    return state;
  if (!groupStat())
    state = MQ_ERR;
  return state;
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

unsigned long NNTPMailbox::msgNew() { return 0; }
unsigned long NNTPMailbox::msgOld() { return 0; }
unsigned long NNTPMailbox::msgFlagged() { return 0; }
unsigned long NNTPMailbox::msgTotal() { return total; }
