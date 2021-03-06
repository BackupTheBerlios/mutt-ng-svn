/**
 * @ingroup libmuttng
 * @addtogroup libmuttng_cache Caching
 * @{
 */
/**
 * @file libmuttng/cache/cache.cpp
 * @brief Implementation: Cache base class
 *
 * This file is published under the GNU General Public License.
 */
#include "cache.h"

#include "../libmuttng_features.h"
#include "libmuttng/config/config_manager.h"

#ifdef LIBMUTTNG_CACHE_QDBM
#include "cache_qdbm.h"
#endif

Cache::Cache (void) {}
Cache::~Cache (void) {}

Cache* Cache::create() {
#ifdef LIBMUTTNG_CACHE_QDBM
  return new QDBMCache();
#endif
  return NULL;
}

bool Cache::getVersion(buffer_t* dst) {
#ifdef LIBMUTTNG_CACHE_QDBM
  return QDBMCache::getVersion(dst);
#endif
  return false;
}

void Cache::reg() {
  ConfigManager::regFeature("cache");
#ifdef LIBMUTTNG_CACHE_QDBM
  ConfigManager::regFeature("qdbm");
#endif
}

/** @} */
