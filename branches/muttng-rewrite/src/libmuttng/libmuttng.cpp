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

#include "config/config_manager.h"

/** static debug obj for library classes */
static Debug* debugObj = NULL;

static int DebugLevel = 0;

LibMuttng::LibMuttng (const char* dir, int u) {
  if (!debugObj) {
    debugObj = new Debug (dir, NULL, u);

    Option* d = new IntOption("debug_level","0",&DebugLevel);
    connectSignal(d->sigOptionChange,this,&LibMuttng::setDebugLevel);
    ConfigManager::reg(d);
    ConfigManager::reg(new StringOption("send_charset","us-ascii:iso-8859-1:iso-8859-15:utf-8",&SendCharset));

#ifdef LIBMUTTNG_POP3
    POP3Mailbox::reg();
#endif

  }
  LibMuttng::debug = debugObj;
}

LibMuttng::~LibMuttng (void) {}

bool LibMuttng::setDebugLevel (const char* name) {
  (void)name;
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
