/** @ingroup libmuttng */
/**
 * @file libmuttng/abstract_class.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Abstract base class.
 */
#include "abstract_class.h"

AbstractClass::AbstractClass (void) {
  this->fd = -1;
}

AbstractClass::~AbstractClass (void) {}

void AbstractClass::debugSetLevel (int level) {
  (void) level;
}

bool AbstractClass::debugStart (void) {
  return (true);
}

bool AbstractClass::debugEnd (void) {
  return (true);
}

bool AbstractClass::debugPrint (const char* file, int line, const char* func,
                                const char* msg) {
  (void) file;
  (void) line;
  (void) func;
  (void) msg;
  return (true);
}

bool AbstractClass::debugPrint (const char* file, int line, const char* func,
                                int num) {
  (void) file;
  (void) line;
  (void) func;
  (void) num;
  return (true);
}
