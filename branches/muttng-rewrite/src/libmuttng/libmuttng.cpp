#include <iostream>
#include "libmuttng.h"

/** static debug obj for library classes */
static Debug* debugObj = NULL;

LibMuttng::LibMuttng (const char* dir, int u) {
  if (!debugObj)
    debugObj = new Debug (dir, NULL, u);
  LibMuttng::debug = debugObj;
}

LibMuttng::~LibMuttng (void) {}

bool LibMuttng::setDebugLevel (int level) {
  DEBUGPRINT(1,("set level to %d", level));
  return (debugObj->setLevel (level));
}

void LibMuttng::cleanup (void) {
  if (debugObj)
    delete (debugObj);
}
