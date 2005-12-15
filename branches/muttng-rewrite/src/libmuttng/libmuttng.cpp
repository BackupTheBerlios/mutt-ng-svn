/** @ingroup libmuttng */
/**
 * @file libmuttng/libmuttng.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: Library base class
 */
#include <iostream>

#include <sys/types.h>
#include <sys/utsname.h>
#include <pwd.h>
#include <unistd.h>
#include <time.h>

#define LIBMUTTNG_MAIN_CPP      1
#include "libmuttng.h"

#include "libmuttng_features.h"

#ifdef LIBMUTTNG_POP3
#include "mailbox/pop3_mailbox.h"
#endif
#ifdef LIBMUTTNG_NNTP
#include "mailbox/nntp_mailbox.h"
#endif

#include "core/version.h"
#include "core/str.h"
#include "core/mem.h"
#include "core/net.h"

#include "libmuttng/version.h"
#include "message/subject_header.h"
#include "config/config_manager.h"

/** static debug obj for library classes */
static Debug* debugObj = NULL;
/** storage for @ref option_debug_level */
static int DebugLevel = 0;

/** get info about user */
static void get_userinfo() {
  char* p = NULL;
  struct passwd* pw;

  if ((p = getenv("HOME")))
    Homedir = str_dup(p);
  if ((pw = getpwuid(getuid()))) {
    Username = str_dup(pw->pw_name);
    Shell = str_dup(pw->pw_shell);
    if (!Homedir)
      Homedir = str_dup(pw->pw_dir);
    /** @bug port mutt_gecos_name() somewhere and use here */
    endpwent();
  } else {
    if ((p = getenv("USER")))
      Username = str_dup(p);
    Shell = str_dup((p = getenv("SHELL"))?p:"/bin/sh");
  }
}

/** get info about host */
static void get_hostinfo() {
  struct utsname utsname;
  char* p;
  buffer_t domain;

  uname(&utsname);
  buffer_init(&domain);
  buffer_init(&Fqdn);

  OSName = str_dup(utsname.sysname);

  if ((p = strchr(utsname.nodename,'.'))) {
    Hostname = str_substrdup(utsname.nodename,p++);
    buffer_add_str(&domain,p,-1);
  } else
    Hostname = str_dup(utsname.nodename);

#ifndef LIBMUTTNG_DOMAIN
#define LIBMUTTNG_DOMAIN domain.str
  if (!p && !net_dnsdomainname(&domain)) {
      Fqdn.size = 2;
      Fqdn.len = 1;
      Fqdn.str = (char*)mem_malloc(2);
      Fqdn.str[0] = '@';
      Fqdn.str[1] = '\0';
  } else
#endif
  if (*LIBMUTTNG_DOMAIN != '@') {
    buffer_add_str(&Fqdn,NONULL(Hostname),-1);
    buffer_add_ch(&Fqdn,'.');
    buffer_add_str(&Fqdn,NONULL(LIBMUTTNG_DOMAIN),-1);
  }
  else
    buffer_add_str(&Fqdn,NONULL(Hostname),-1);

  buffer_free(&domain);
}

LibMuttng::LibMuttng () {
  if (!debugObj) {

    /* before we have all library-internal options, get other stuff */
    get_userinfo();
    get_hostinfo();

    debugObj = new Debug (Homedir, NULL, Umask);

    /* register all options for library globally */
    Option* d = new IntOption("debug_level","0",&DebugLevel,0,5);
    connectSignal<LibMuttng,Option*>(d->sigOptionChange,this,
                                     &LibMuttng::setDebugLevel);
    ConfigManager::reg(d);
    ConfigManager::reg(new StringOption("send_charset",
                                        "us-ascii:iso-8859-1:iso-8859-15:utf-8",
                                        &SendCharset));

    ConfigManager::reg(new SysOption("muttng_core_version",CORE_VERSION));
    ConfigManager::reg(new SysOption("muttng_libmuttng_version",
                                     LIBMUTTNG_VERSION));
    ConfigManager::reg(new SysOption("muttng_hostname",Hostname));
    ConfigManager::reg(new SysOption("muttng_system",OSName));
    ConfigManager::reg(new SysOption("muttng_dnsname",Fqdn.str));

    /* register all options within library modules */
#ifdef LIBMUTTNG_POP3
    POP3Mailbox::reg();
#endif
#ifdef LIBMUTTNG_NNTP
    NNTPMailbox::reg();
#endif
    SubjectHeader::reg();

    buffer_init(&AttachMarker);
    buffer_add_str(&AttachMarker,"\033]9;",3);
    buffer_add_num(&AttachMarker,(int)time(NULL),-1);
    buffer_add_ch(&AttachMarker,'\a');

  }
  LibMuttng::debug = debugObj;
}

LibMuttng::~LibMuttng (void) {}

bool LibMuttng::setDebugLevel (Option* option) {
  (void) option;
  return setDebugLevel(DebugLevel);
}

bool LibMuttng::setDebugLevel (int level) {
  DEBUGPRINT(D_MOD,("set level to %d", level));
  return (debugObj->setLevel (level));
}

int LibMuttng::getDebugLevel() { return DebugLevel; }

void LibMuttng::cleanup (void) {
  if (debugObj)
    delete (debugObj);
  mem_free(&Homedir);
  mem_free(&Realname);
  mem_free(&Shell);
  mem_free(&Username);
}
