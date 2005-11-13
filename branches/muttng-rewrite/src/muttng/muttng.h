#ifndef MUTTNG_MUTTNG_H
#define MUTTNG_MUTTNG_H

#include "libmuttng/debug.h"

#include "event/event.h"

/**
 * Base class for all classes of muttng to have application-wide
 * debugging.
 */
class Muttng {
  public:
    Muttng (void);
    ~Muttng (void);
    bool setDebugLevel (int level);
    void muttngInit (const char* dir, const char* prefix, int u);
  protected:
    Debug* debug;
    Event* event;
};

#endif /* !MUTTNG_MUTTNG_H */
