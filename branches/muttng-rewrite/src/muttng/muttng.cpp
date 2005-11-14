#include <iostream>

#include "muttng.h"

static Debug* debugObj = NULL;
static Event* eventObj = NULL;

Muttng::Muttng (void) {
  Muttng::debug = debugObj;
  Muttng::event = eventObj;
}

Muttng::~Muttng (void) {}

void Muttng::muttngInit (const char* dir, const char* prefix, int u) {
  debugObj = new Debug (dir, prefix, u);
  eventObj = new Event (debugObj);
  Muttng::debug = debugObj;
  Muttng::event = eventObj;
}

void Muttng::muttngCleanup (void) {
  if (eventObj)
    delete (eventObj);
  if (debugObj)
    delete (debugObj);
}

bool Muttng::setDebugLevel (int level) {
  return (debug->setLevel (level));
}
