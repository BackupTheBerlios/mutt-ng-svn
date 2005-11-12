#ifndef LIBMUTTNG_LIBMUTTNG_H
#define LIBMUTTNG_LIBMUTTNG_H

#include "debug.h"

/**
 * Base class for all classes of libmuttng to have library-wide
 * debugging.
 */
class LibMuttng {
  public:
    LibMuttng (const char* dir = NULL, int u = -1);
    ~LibMuttng (void);
    bool setDebugLevel (int level);
  protected:
    Debug* debug;
};

#endif /* !LIBMUTTNG_LIBMUTTNG_H */
