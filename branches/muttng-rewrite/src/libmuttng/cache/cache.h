/** @ingroup libmuttng_cache */
/**
 * @file libmuttng/cache/cache.h
 * @brief Interface: Cache base class
 *
 * This file is published under the GNU General Public License.
 */
#ifndef LIBMUTTNG_CACHE_CACHE_H
#define LIBMUTTNG_CACHE_CACHE_H

#include "core/buffer.h"

#include "libmuttng/libmuttng.h"
#include "libmuttng/muttng_signal.h"
#include "libmuttng/message/message.h"
#include "libmuttng/util/url.h"

/** Caching base class */
class Cache : public LibMuttng {
  public:
    /** constructor */
    Cache (void);
    /** destructor */
    virtual ~Cache(void) = 0;
    /**
     * Load single message from cache.
     * This is to be implemented by the various caching modules.
     * @param url URL of mailbox.
     * @param key The key for the message to load.
     * @return If message could be restored, it's returned
     *         and @c NULL otherwise.
     */
    virtual Message* cacheLoadSingle (url_t* url, const char* key) = 0;
    /**
     * Store single message from cache.
     * This is to be implemented by the various caching modules.
     * @param url URL of mailbox.
     * @param key The key for the message to load.
     * @param message The message to store.
     * @return Whether storing succeeded.
     */
    virtual bool cacheDumpSingle (url_t* url, const char* key, Message* message) = 0;
    /**
     * Get caching module
     * @return pointer to module.
     */
    static Cache* create();

    /** register all caching specific stuff */
    static void reg();

    /**
     * Get version of caching module.
     * @param dst Optional destination buffer.
     * @return Wether caching is enabled.
     */
    static bool getVersion(buffer_t* dst);
    /**
     * Signal emitted when we need a key.
     * This is caught by a Mailbox class with caching.
     * The parameters passed are:
     * -# the message to compute key for
     * -# destination where to store key
     */
    Signal2<Message*,buffer_t*> cacheGetKey;
};

#endif /* !LIBMUTTG_CACHE_CACHE_H */
