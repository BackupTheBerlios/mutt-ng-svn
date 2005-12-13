/**
 * @ingroup muttng
 * @addtogroup muttng_bin Binaries
 * @{
 */
/**
 * @file muttng/tools/tool.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: Common tool base class
 */
#ifndef MUTTNG_TOOL_H
#define MUTTNG_TOOL_H

#include <stdlib.h>

#include "core/buffer.h"

#include "libmuttng/libmuttng.h"
#include "libmuttng/config/option.h"

#include "muttng.h"
#include "ui/ui.h"

/** generic command-line arguments common for all tools. */
#define GENERIC_ARGS    "vVhnF:d:"

/**
 * Abstract superclass for building binaries with a user interface based
 * on the core and libmuttng.
 */
class Tool : public Muttng {
  public:
    /**
     * Constructor.
     * Intialize a tool with a given user interface and
     * command-line arguments.
     * @param argc @c argc from @c main().
     * @param argv @c argv from @c main().
     */
    Tool (int argc = 0, char** argv = NULL);
    /** desctructor */
    virtual ~Tool (void);
    /**
     * Run a tool.
     * @return Exit status for @c main().
     */
    virtual int main (void) = 0;
    /**
     * Get the name of the tool.
     * @return Name.
     */
    virtual const char* getName (void) = 0;
    /** Display version info using the UI. */
    void displayVersion (void);
    /** Display version info using the UI. */
    void displayWarranty (void);
    /** Display version info using the UI. */
    void displayUsage (void);
    /**
     * Query the actual tool implementation for a usage string.
     * @param dst Destination buffer for string.
     */
    virtual void getUsage (buffer_t* dst) = 0;
  protected:
    /**
     * For use by inheriting classes: see if some argument is a generic
     * one for all tools. This already does handle it.
     * @param c Command-line switch.
     * @param arg Optional argument for switch.
     * @return
     *   - @c 1: argument known, continue
     *   - @c 0: argument known, we did action, exit with 0
     *   - @c -1: argument unknown, exit with 1
     */
    int genericArg (unsigned char c, const char* arg);
    /**
     * Start after command-line handling.
     * This is supposed to be called from Tool::main().
     */
    bool start (void);
    /**
     * Finish when done.
     * This is supposed to be called from Tool::main().
     */
    bool end (void);
    /** @c argc from @c main. */
    int argc;
    /** @c argv from @c main. */
    char** argv;
    /** the UI to use */
    UI* ui;
    /** whether to read global config file */
    bool readGlobal;
    /** alternative config to read */
    const char* altConfig;
    /** lib object */
    LibMuttng* libmuttng;
  private:
    /**
     * Compose a string with name and version info.
     * @param dst Desination buffer for string.
     */
    void doName (buffer_t* dst);
    /**
     * Compose a string with legal info.
     * @param dst Desination buffer for string.
     */
    void doCopyright (buffer_t* dst);
    /**
     * Compose a string with info how to contact us.
     * @param dst Desination buffer for string.
     */
    void doContact (buffer_t* dst);
    /**
     * Compose a string with system and compile-time info.
     * @param dst Desination buffer for string.
     */
    void doSystem (buffer_t* dst);
    /**
     * Compose a string with license and warranty info.
     * @param dst Desination buffer for string.
     */
    void doLicense (buffer_t* dst);
    /** setup all event handlers */
    void setupEventHandlers (void);

    /**
     * Signal connected to IntOption::sigOptionChange for $debug_level.
     * @param option Option for $debug_level.
     * @return Success of changing level.
     */
    bool catchDebugLevelChange (Option* option);

    /**
     * Catch any context change.
     * Connected to Event::sigContextChange. This is currently only used
     * for testing.
     * @param context New or old context we go to/leave.
     * @param event Which type of change: Event::E_CONTEXT_ENTER,
     *              Event::E_CONTEXT_REENTER or E_CONTEXT_LEAVE.
     * @return @c true
     */
    bool catchContextChange (Event::context context, Event::event event);
};

#endif /* !MUTTNG_TOOL_H */

/** @} */
