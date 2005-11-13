/** @ingroup libmuttng */
/**
 * @file libmuttng/libmuttng.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Library base class interface
 */
#ifndef LIBMUTTNG_LIBMUTTNG_H
#define LIBMUTTNG_LIBMUTTNG_H

#include "debug.h"

/**
 * Base class for all classes of libmuttng to have library-wide
 * debugging.
 */
class LibMuttng {
  public:
    /**
     * Constructor.
     * @param dir Directory for debug files
     * @param u Umask for debug files.
     */
    LibMuttng (const char* dir = NULL, int u = -1);
    /** destructor */
    ~LibMuttng (void);
    /**
     * Adjust debug level.
     * @param level New debug level.
     * @return Success.
     */
    bool setDebugLevel (int level);
    /** Cleanup after use of library. */
    void cleanup (void);
  protected:
    /** library-wide debug object */
    Debug* debug;
};

#endif /* !LIBMUTTNG_LIBMUTTNG_H */
