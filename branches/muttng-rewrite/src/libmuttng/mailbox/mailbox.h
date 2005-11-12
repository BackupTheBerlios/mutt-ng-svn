/**
 * @ingroup libmuttng
 * @addtogroup libmuttng_mailbox Mailbox
 * @{
 */
/**
 * @file mailbox.h
 * @author Someone
 * @version $Id$
 * @brief Mailbox interface.
 */
#include "libmuttng.h"

/**
 * Base class for mailbox abstraction.
 */
class Mailbox : public LibMuttng {
  public:
    /** construct empty mailbox */
    Mailbox ();
    /** construct mailbox form URL already */
    Mailbox (const char* url);
    /** cleanup */
    ~Mailbox ();
    /**
     * Get URL for mailbox.
     * @return URL or @c NULL if none set (yet.)
     */
    const char* getUrl ();
  protected:
    /** whether mailbox supports caching */
    unsigned int haveCaching:1;
    /** whether mailbox supports authencation */
    unsigned int haveAuthentication:1;
    /** whether mailbox supports encryption */
    unsigned int haveEncryption:1;
    /** whether mailbox supports filters */
    unsigned int haveFilters:1;
    /** URL if any. */
    const char* url;
};

/** @} */
