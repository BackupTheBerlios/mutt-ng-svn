/** @ingroup libmuttng */
/**
 * @file libmuttng/libmuttng.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: Library base class
 */
#include <iostream>

#define LIBMUTTNG_MAIN_CPP      1
#include "libmuttng.h"

#include "libmuttng_features.h"

#ifdef LIBMUTTNG_POP3
#include "mailbox/pop3_mailbox.h"
#endif

#include "core/version.h"

#include "libmuttng/version.h"
#include "message/subject_header.h"
#include "config/config_manager.h"

/** static debug obj for library classes */
static Debug* debugObj = NULL;
/** storage for @ref option_debug_level */
static int DebugLevel = 0;

LibMuttng::LibMuttng (const char* dir, int u) {
  if (!debugObj) {
    debugObj = new Debug (dir, NULL, u);

    Option* d = new IntOption("debug_level","0",&DebugLevel,0,5);
    connectSignal<LibMuttng,Option*>(d->sigOptionChange,this,&LibMuttng::setDebugLevel);
    ConfigManager::reg(d);
    ConfigManager::reg(new StringOption("send_charset","us-ascii:iso-8859-1:iso-8859-15:utf-8",&SendCharset));

    ConfigManager::reg(new SysOption("muttng_core_version",CORE_VERSION));
    ConfigManager::reg(new SysOption("muttng_libmuttng_version",LIBMUTTNG_VERSION));

#ifdef LIBMUTTNG_POP3
    POP3Mailbox::reg();
#endif
    SubjectHeader::reg();
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
}
