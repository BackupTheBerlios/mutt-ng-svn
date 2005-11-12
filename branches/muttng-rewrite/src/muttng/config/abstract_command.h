/**
 * @file muttng/config/abstract_command.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Configuration command superclass interface
 */
#ifndef MUTTNG_CONFIG_ABSTRACT_COMMAND_H
#define MUTTNG_CONFIG_ABSTRACT_COMMAND_H

#include "core/buffer.h"

#include "muttng.h"
#include "ui/ui.h"
#include "ui/config_screen.h"

/**
 * Abstract superclass for configuration commands.
 */
class AbstractCommand : public Muttng {
  public:
    /** constructor */
    AbstractCommand (void);
    /** destructor */
    virtual ~AbstractCommand (void) = 0;
    /**
     * Handler to be overwritten by inheriting class.
     * @bug I really want to have this <code>virtual static</code>.
     * @param line Config line.
     * @param error Where to put error messages.
     * @param data Arbitraty data.
     * @return Whether line is valid.
     */
    virtual bool handle (buffer_t* line, buffer_t* error, unsigned long data) = 0;
    /**
     * Print all known items for a command.
     * @param configScreen Destination.
     * @param changedOnly If @c true, print only changed ones.
     * @param annotated If @c true and value differs from default,
     *                  print default, too.
     * @return Success.
     */
    virtual bool print (ConfigScreen* configScreen, bool changedOnly = false,
                        bool annotated = false) = 0;
    /**
     * Init for a command.
     * @param ui User interface for error reporting.
     * @return Success.
     */
    virtual bool init (UI* ui) = 0;
};

#endif /* !MUTTNG_CONFIG_ABSTRACT_COMMAND_H */
