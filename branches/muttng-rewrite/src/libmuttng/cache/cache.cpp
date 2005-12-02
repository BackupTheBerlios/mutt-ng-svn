/**
 * @ingroup libmuttng
 * @addtogroup libmuttng_cache Caching
 * @{
 */
/**
 * @file libmuttng/cache/cache.cpp
 * @brief Implementation: Cache base class
 */
#include "cache.h"

#include "muttng_features.h"

#if LIBMUTTNG_CACHE_QDBM
#include "cache_qdbm.h"
#endif

Cache::Cache (void) {}
Cache::~Cache (void) {}

Cache* Cache::create() {
#if LIBMUTTNG_CACHE_QDBM
  return new QDBMCache();
#endif
  return NULL;
}

/** @} */
