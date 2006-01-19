/** @ingroup libmuttng_mailbox */
/**
 * @file libmuttng/mailbox/local_mailbox.h
 * @brief Interface: Local Mailbox base class
 *
 * This file is published under the GNU General Public License.
 */
#ifndef LIBMUTTNG_MAILBOX_LOCAL_MAILBOX_H
#define LIBMUTTNG_MAILBOX_LOCAL_MAILBOX_H

#include "libmuttng/mailbox/mailbox.h"

#include <sys/types.h>
#include <sys/stat.h>

/**
 * Mailbox with an underlaying local storage.
 */
class LocalMailbox : public Mailbox {
  public:
    /**
     * Create new local mailbox from URL.
     * @param url_ url.
     */
    LocalMailbox (url_t* url_);
    ~LocalMailbox ();

    /**
     * Create local mailbox from URL.
     * @param url_ URL.
     * @param error Error buffer.
     * @return Mailbox or @c NULL in case of error.
     */
    static Mailbox* fromURL (url_t* url_,buffer_t* error);

    /**
     * Register variables specific to local mailboxes.
     */
    static void reg();

};

#endif
