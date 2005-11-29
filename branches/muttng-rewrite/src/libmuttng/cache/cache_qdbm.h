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
    ~QDBMCache(void);
    Message* cacheLoadSingle (url_t* url, const char* key);
    bool cacheDumpSingle (url_t* url, const char* key, Message* message);
};

#endif /* !LIBMUTTG_CACHE_CACHE_QDBM_H */
