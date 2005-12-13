/* doxygen documentation {{{ */
/**
 * @addtogroup libmuttng libmuttng
 * @{
 *
 *   @section libmuttng_intro Introduction
 *
 *     This is a library containing the e-mail logic. It only depends on
 *     the core and thus can be re-used easily. It consists of the
 *     following parts:
 *
 *       - @b mailbox Support for the following types of folders: IMAP,
 *         POP, NNTP, Maildir, MH, MBOX and MMDF. The abstraction layer
 *         is URL-driven, so that for applications the whole handling is
 *         fully transparent. For remote folders, features like
 *         authentication and encryption are supported, too.
 *       - @b cache Support for transparent caching of certain mailbox
 *         types: IMAP, NNTP, Maildir and MH.
 *       - @b crypto Support for PGP/GnuPG and S/MIME. Either gpgme or
 *         locally installed tools such as gpg/pgp and openssl can be
 *         used.
 *       - @b mime Fully functional, transparent and modular
 *         MIME-handling.
 *
 *   @section libmuttng_mailbox Mailbox handling
 *
 *     The mailbox handling part is URL-driven to achieve transparency
 *     for applications.
 *
 *   @section libmuttng_cache Caching
 *
 *   @section libmuttng_crypto Cryptography
 *
 *   @section libmuttng_mime MIME handling
 */
/** @defgroup libmuttng_util Utility classes */
/* }}} */
/**
 * @file libmuttng/libmuttng.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: Library base class
 */
#ifndef LIBMUTTNG_LIBMUTTNG_H
#define LIBMUTTNG_LIBMUTTNG_H

#include "libmuttng/debug.h"
#include "libmuttng/config/option.h"

/** module init/cleanup debug level */
#define D_MOD           1
/** parser debug level */
#define D_PARSE         2
/** socket debug level */
#define D_SOCKET        5

/**
 * Base class for all classes of libmuttng to have library-wide
 * debugging.
 */
class LibMuttng {
  public:
    /**
     * Constructor.
     * @param dir Directory for debug files
     * @param u Umask for debug files.
     */
    LibMuttng (const char* dir = NULL, int u = -1);
    /** destructor */
    ~LibMuttng (void);
    /**
     * Adjust debug level.
     * @param level Level in [0,5].
     * @return Success.
     */
    bool setDebugLevel (int level);
    /**
     * Get debug level.
     * As $debug_level is stored in the library but the apps may want
     * to use it as well, we catch a change in the apps and need to
     * set the new level for the app. However, the apps don't have
     * access to the storage of DebugLevel so they need this method.
     * @return Level.
     */
    int getDebugLevel();
    /** Cleanup after use of library. */
    void cleanup (void);
  protected:
    /**
     * Signal handler catching changes of $debug_level.
     * @param option $debug_level.
     * @return Success of setting level.
     */
    bool setDebugLevel (Option* option);
    /** library-wide debug object */
    Debug* debug;
};

#ifdef WHERE
#undef WHERE
#endif

#ifdef LIBMUTTNG_MAIN_CPP
#define WHERE
#else
#define WHERE extern
#endif

/** storage for $send_charset */
WHERE char* SendCharset;

#endif /* !LIBMUTTNG_LIBMUTTNG_H */
/** @} */
