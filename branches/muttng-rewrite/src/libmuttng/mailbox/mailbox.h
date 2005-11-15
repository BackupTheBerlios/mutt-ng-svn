/**
 * @ingroup libmuttng
 * @addtogroup libmuttng_mailbox Mailbox
 * @{
 */
/**
 * @file libmuttng/mailbox/mailbox.h
 * @brief Interface: Mailbox base class
 */
#include "../libmuttng.h"

/**
 * Base class for mailbox abstraction.
 */
class Mailbox : public LibMuttng {
  public:
    /** construct mailbox form URL already */
    Mailbox (const char* url = NULL);
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
