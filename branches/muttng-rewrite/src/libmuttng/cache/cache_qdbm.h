/** @ingroup libmuttng_cache */
/**
 * @file libmuttng/cache/cache_qdbm.h
 * @brief Interface: QDBM-based caching
 */
#ifndef LIBMUTTNG_CACHE_CACHE_QDBM_H
#define LIBMUTTNG_CACHE_CACHE_QDBM_H

#include "cache.h"

/** Caching base class */
class QDBMCache : public Cache {
  public:
    /** constructor */
    QDBMCache (void);
    /** destructor */
    virtual ~QDBMCache(void) = 0;
    void open (void);
    void close (void);
    virtual const char* key (Message* msg) = 0;
};

#endif /* !LIBMUTTG_CACHE_CACHE_QDBM_H */
