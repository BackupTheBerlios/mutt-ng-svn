/** @ingroup libmuttng */
/**
 * @file libmuttng/debug.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: Basic debug support
 *
 * This file is published under the GNU General Public License.
 */
#ifndef LIBMUTTNG_ABSTRACT_CLASS_H
#define LIBMUTTNG_ABSTRACT_CLASS_H

#include <stdio.h>

#include "core/buffer.h"

/** maximum number of IDs to try for filename generation. */
#define MAX_DBG_FILES           20
/** minimum debug level */
#define DEBUG_MIN               0
/** maxmimum debug level */
#define DEBUG_MAX               5
/** module init/cleanup debug level (plus basics) */
#define D_MOD                   1
/** parser debug level */
#define D_PARSE                 2
/** default signal/event level */
#define D_EVENT                 3
/** socket debug level */
#define D_SOCKET                5

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
    /** spaces for indentation */
    char sp[((DEBUG_MAX-1)*2)+2];
};

#ifdef __GNUG__
#define DEBUGPRINT2(O,L,X) do { \
  if (O->printIntro (__FILE__,__LINE__,__FUNCTION__,L)) \
    O->printLine X; \
} while (0)

#else
#define DEBUGPRINT2(O,L,X) do { \
  if (O->printIntro (__FILE__,__LINE__,NULL,L)) \
    O->printLine X; \
} while (0)
#endif

/**
 * @def DEBUGPRINT2
 * Compiler-dependent: debug print macro.
 * Use with classes derived from Muttng or LibMuttng only.
 * @param O Who wants debugging.
 * @param L Message's debug level.
 * @param X @c printf()-style message in brackets,
 *          eg <code>("foo %s", bar)</code>
 */

/**
 * Print a debug line.
 * @param L Level.
 * @param X @c printf()-style message in brackets,
 *          eg <code>("foo %s", bar)</code>
 */
#define DEBUGPRINT(L,X) DEBUGPRINT2(this->debug,L,X)

#endif /* !LIBMUTTNG_ABSTRACT_CLASS_H */
