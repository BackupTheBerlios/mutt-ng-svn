#include <iostream>

#include "muttng.h"

Muttng::Muttng (void) {
  Muttng::debug = NULL;
  Muttng::event = NULL;
}

Muttng::~Muttng (void) {
  if (Muttng::debug)
    delete (Muttng::debug);
  if (Muttng::event)
    delete (Muttng::event);
}

void Muttng::muttngInit (const char* dir, const char* prefix, int u) {
  Muttng::debug = new Debug (dir, prefix, u);
  Muttng::event = new Event (Muttng::debug);
}

bool Muttng::setDebugLevel (int level) {
  return (Muttng::debug->setLevel (level));
}
