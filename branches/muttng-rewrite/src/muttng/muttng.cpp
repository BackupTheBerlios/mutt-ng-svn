/** @ingroup muttng */
/**
 * @file muttng/muttng.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: Application base class
 */
#include <iostream>

#include "muttng.h"

/** application-wide debug object */
static Debug* debugObj = NULL;
/** application-wide event handler */
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
