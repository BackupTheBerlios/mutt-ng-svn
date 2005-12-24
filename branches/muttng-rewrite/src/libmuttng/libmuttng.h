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
     */
    LibMuttng ();
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

    /**
     * Signal emitted when some part wants to display progress messages.
     * Parameters are in order:
     * -# message
     */
    static Signal1<const buffer_t*> displayProgress;

    /**
     * Signal emitted when some part wants to display informational
     * messages and wants to have the user see it. This slightly differs
     * from displayProgress signal as this one is more important.
     * Parameters are in order:
     * -# message
     */
    static Signal1<const buffer_t*> displayMessage;

    /**
     * Signal emitted when some part wants to display an error message.
     * Parameters are in order:
     * -# message
     */
    static Signal1<const buffer_t*> displayError;

    /**
     * Signal emitted when some part wants to display a warning message.
     * Parameters are in order:
     * -# message
     */
    static Signal1<const buffer_t*> displayWarning;

  protected:
    /**
     * Signal handler catching changes of $debug_level.
     * @param option $debug_level.
     * @return Success of setting level.
     */
    bool setDebugLevel (Option* option);
    /** library-wide debug object */
    Debug* debug;
  private:
    /** handler for displayError signal: print text to debug file */
    bool debugError(const buffer_t* msg);
    /** handler for displayWarning signal: print text to debug file */
    bool debugWarning(const buffer_t* msg);
    /** handler for displayMessage signal: print text to debug file */
    bool debugMessage(const buffer_t* msg);
    /** handler for displayProgress signal: print text to debug file */
    bool debugProgress(const buffer_t* msg);
};

/**
 * @def WHERE
 * Neat trick to have storage+visibility of options: define storage location.
 */
#ifdef WHERE
#undef WHERE
#endif

/**
 * @def INITVAL(X).
 * Neat trick to have storage+visibility of options: set initial value.
 * @param X Inititial value.
 */
#ifdef INITVAL
#undef INITVAL
#endif

#ifdef LIBMUTTNG_MAIN_CPP
#define WHERE
#define INITVAL(X)      =X
#else
#define WHERE extern
#define INITVAL(X)
#endif

/*
 * info about environment
 */

/** user's home directory */
WHERE char* Homedir INITVAL(NULL);
/** user's realname */
WHERE char* Realname INITVAL(NULL);
/** user's shell */
WHERE char* Shell INITVAL(NULL);
/** user's login */
WHERE char* Username INITVAL(NULL);
/** hostname */
WHERE char* Hostname INITVAL(NULL);
/** fqdn */
WHERE buffer_t Fqdn;
/** OS name */
WHERE char* OSName INITVAL(NULL);

/*
 * Misc.
 */

/**
 * magic sequence to prevent attacks embedding them in incomin messages
 * @bug move out to mime decoder/body filters/whoever is going to use it
 */
WHERE buffer_t AttachMarker;

/*
 * Library options
 */

/** storage for $send_charset */
WHERE char* SendCharset INITVAL(NULL);
/** storage for $charset */
WHERE char* Charset INITVAL(NULL);
/** storage for $umask */
WHERE int Umask INITVAL(0);

#endif /* !LIBMUTTNG_LIBMUTTNG_H */
/** @} */
