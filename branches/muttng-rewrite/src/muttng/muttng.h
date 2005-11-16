/**
 * @addtogroup muttng Muttng
 * @{
 */
/**
 * @file muttng/muttng.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: Application base class
 */
#ifndef MUTTNG_MUTTNG_H
#define MUTTNG_MUTTNG_H

#include "libmuttng/debug.h"

#include "event/event.h"

/** module init/cleanup debug level (plus basics) */
#define D_MOD           1
/** parser debug level */
#define D_PARSE         2
/** event handler debug level */
#define D_EVENT         3
/** socket debug level */
#define D_SOCKET        5

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
    /**
     * Cleanup after run.
     * E.g. shutdown event handler, shutdown debgging, etc.
     */
    void muttngCleanup (void);
  protected:
    /** application-wide debug object */
    Debug* debug;
    /** application-wide event object */
    Event* event;
};

#endif /* !MUTTNG_MUTTNG_H */

/** @} */
