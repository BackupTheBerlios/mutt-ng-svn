/** @ingroup libmuttng_cache */
/**
 * @file libmuttng/cache/cache.h
 * @brief Interface: Cache base class
 */
#ifndef LIBMUTTNG_CACHE_CACHE_H
#define LIBMUTTNG_CACHE_CACHE_H

#include "../libmuttng.h"
#include "../message/message.h"

/** Caching base class */
class Cache : public LibMuttng {
  public:
    /** constructor */
    Cache (void);
    /** destructor */
    virtual ~Cache(void) = 0;
    /** dummy */
    virtual void open (void) = 0;
    /** dummy */
    virtual void close (void) = 0;
    /**
     * Compute key for message.
     * This must be implmented by Mailbox classes.
     * @param msg Message.
     * @return Key.
     */
    virtual const char* key (Message* msg) = 0;
};

#endif /* !LIBMUTTG_CACHE_CACHE_H */
