/** @ingroup libmuttng */
/**
 * @file libmuttng/debug.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Abstract base class.
 */
#ifndef LIBMUTTNG_ABSTRACT_CLASS_H
#define LIBMUTTNG_ABSTRACT_CLASS_H

#include <stdio.h>

#include "core/buffer.h"

/**
 * Library-wide debug interface. It's recommended to use the
 * DEBUGPRINT() macro only instead of Debug::print() and
 * Debug::intro().
 *
 * The files created by these are by default:
 * <code>$dir/.$prefix.$pid.id.log</code>
 *
 * whereby:
 *
 *  - $dir and $prefix come via Debug::init(). If none given, the
 *    default for dir is $PWD and the default for $prefix is
 *    "libmuttng".
 *  - $pid is the current process' PID
 *  - id is some number to avoid conflicts with existing files
 */
class Debug {
  public:
    /**
     * Constructor.
     * @param dir Where debug file will be placed.
     * @param prefix How debug file will be named.
     * @param u Umask for files created.
     */
    Debug (const char* dir = NULL, const char* prefix = NULL, int u = -1);
    /** destructor */
    ~Debug (void);
    /**
     * Change debug level.
     * If current level is invalid and new is valid, do start().
     * If current level is valid and new is invalid, do end().
     * @param level New level.
     */
    bool setLevel (int level);
    /**
     * Print line prefix.
     * <b>DO NOT USE</b>.
     * @param file @c __FILE__
     * @param line @c __LINE__
     * @param function @c __FUNCTION__
     * @param level Debug level.
     * @return Whether line may be printed.
     * @sa DEBUGPRINT()
     */
    bool printIntro (const char* file, int line,
                     const char* function, int level);
    /**
     * Print line.
     * <b>DO NOT USE</b>.
     * @param fmt Format string.
     */
    void printLine (const char* fmt, ...);
  private:
    /** output file pointer */
    FILE* fp;
    /** directory */
    buffer_t dir;
    /** prefix */
    buffer_t prefix;
    /** current level */
    int level;
    /** umask */
    int u;
    /** Start debugging: open debug file. */
    bool start (void);
    /** Finish debugging: close debug file. */
    bool end (void);
};

#ifdef __GNUG__
#define DEBUGPRINT(L,X) do { \
  if (this->debug->printIntro (__FILE__,__LINE__,__FUNCTION__,L)) \
    this->debug->printLine X; \
} while (0)
#else
#define DEBUGPRINT(L,X) do { \
  if (this->debug->printIntro (__FILE__,__LINE__,NULL,L)) \
    this->debug->printLine X; \
} while (0)
#endif

#endif /* !LIBMUTTNG_ABSTRACT_CLASS_H */
