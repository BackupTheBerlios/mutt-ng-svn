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
    /**
     * Set application-wide debug level.
     * @param level Level.
     * @return Success.
     */
    bool setDebugLevel (int level);
    /**
     * Initialize debugging.
     * @param dir Directory for Debug.
     * @param prefix Prefix for Debug.
     * @param u Umask for Debug.
     */
    void muttngInit (const char* dir, const char* prefix, int u);
    void muttngCleanup (void);
  protected:
    /** application-wide debug object */
    Debug* debug;
    /** application-wide event object */
    Event* event;
};

#endif /* !MUTTNG_MUTTNG_H */
