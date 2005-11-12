#include <iostream>
#include "libmuttng.h"

LibMuttng::LibMuttng (const char* dir, int u) {
  if (!LibMuttng::debug)
    LibMuttng::debug = new Debug (dir, NULL, u);
}

LibMuttng::~LibMuttng (void) {
  if (LibMuttng::debug)
    delete (LibMuttng::debug);
}

bool LibMuttng::setDebugLevel (int level) {
  return (LibMuttng::debug->setLevel (level));
}
