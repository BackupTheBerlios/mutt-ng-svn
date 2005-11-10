/**
 * @file muttng/config/abstract_command.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Configuration command superclass interface
 */
#ifndef MUTTNG_CONFIG_ABSTRACT_COMMAND_H
#define MUTTNG_CONFIG_ABSTRACT_COMMAND_H

#include "core/buffer.h"

/**
 * Abstract superclass for configuration commands.
 */
class AbstractCommand {
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
     * @param dst Destination buffer.
     * @param changedOnly If @c true, print only changed ones.
     * @param annotated If @c true and value differs from default,
     *                  print default, too.
     * @return Success.
     */
    virtual bool print (buffer_t* dst, bool changedOnly = false,
                        bool annotated = false) = 0;
    /**
     * Init for a command.
     */
    virtual void init (void) = 0;
};

#endif /* !MUTTNG_CONFIG_ABSTRACT_COMMAND_H */
