/** @ingroup libmuttng_cache */
/**
 * @file libmuttng/cache/cache_qdbm.cpp
 * @brief Implementation: QDBM-based caching
 *
 * This file is published under the GNU General Public License.
 */
#include "cache_qdbm.h"

//#include <depot.h>

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

bool QDBMCache::getVersion(buffer_t* dst) {
  if (!dst)
    return true;
  buffer_add_str(dst,"qdbm ",5);
//  buffer_add_str(dst,dpversion,-1);
  return true;
}
