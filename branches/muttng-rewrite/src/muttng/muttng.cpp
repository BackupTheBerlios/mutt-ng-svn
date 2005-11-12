#include <iostream>

#include "muttng.h"

Muttng::Muttng (void) {
  Muttng::debug = NULL;
}

Muttng::~Muttng (void) {
  if (Muttng::debug)
    delete (Muttng::debug);
}

void Muttng::muttngInit (const char* dir, const char* prefix, int u) {
  Muttng::debug = new Debug (dir, prefix, u);
}

bool Muttng::setDebugLevel (int level) {
  return (Muttng::debug->setLevel (level));
}
