/** @ingroup libmuttng */
/**
 * @file libmuttng/abstract_class.h
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
 *
 * @bug we need to switch to @c printf() style here.
 */
class Debug {
  public:
    /** constructor */
    Debug ();
    /** destructor */
    ~Debug (void);
    /**
     * Initialize.
     * @param dir Where to put debug file.
     * @param prefix How to prefix filename.
     */
    static void init (const char* dir = NULL, const char* prefix = NULL);
    /**
     * Change debug level.
     * If level is <= 0, do Debug::end().
     * @param level New level.
     */
    static void setLevel (int level);
    /** Start debugging: open debug file. */
    static bool start (void);
    /** Finish debugging: close debug file. */
    static bool end (void);
    /**
     * Print debug line intro.
     * Format: '[file:line:function()] '.
     * @param level Level.
     * @param file Source file.
     * @param line Souce line.
     * @param func Source Function.
     */
    static void intro (int level, const char* file, int line, const char* func);
    /**
     * Print string to debug file.
     * @param level Level.
     * @param msg Message.
     */
    static void print (int level, const char* msg);
    /**
     * Print number to debug file.
     * @param level Level.
     * @param num Number.
     */
    static void print (int level, int num);
};

#ifdef __GNUG__
#define DEBUGPRINT(L,X) do { \
  Debug::intro(L,__FILE__,__LINE__,__FUNCTION__); \
  Debug::print(L,X); \
} while (0);
#else
#define DEBUGPRINT(L,X) do { \
  Debug::intro(L,__FILE__,__LINE__,NULL); \
  Debug::print(L,X); \
} while (0);
#endif

#endif /* !LIBMUTTNG_ABSTRACT_CLASS_H */
