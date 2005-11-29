/** @ingroup libmuttng_cache */
/**
 * @file libmuttng/cache/cache_qdbm.cpp
 * @brief Implementation: QDBM-based caching
 */
#include "cache_qdbm.h"

QDBMCache::QDBMCache (void) {}
QDBMCache::~QDBMCache (void) {}

Message* QDBMCache::cacheLoadSingle (url_t* url, const char* key) {
  (void) url;
  (void) key;
  return (NULL);
}

bool QDBMCache::cacheDumpSingle (url_t* url, const char* key, Message* message) {
  (void) url;
  (void) key;
  (void) message;
  return (false);
}
