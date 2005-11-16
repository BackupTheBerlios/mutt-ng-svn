/** @ingroup muttng_conf */
/**
 * @file muttng/config/abstract_command.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: Configuration command base class
 */
#ifndef MUTTNG_CONFIG_ABSTRACT_COMMAND_H
#define MUTTNG_CONFIG_ABSTRACT_COMMAND_H

#include "core/buffer.h"

#include "muttng.h"
#include "option.h"
#include "ui/ui.h"

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
    virtual bool handle (buffer_t* line, buffer_t* error,
                         unsigned long data) = 0;
    /**
     * Get a single option with current and default value.
     * @b NOTE: All buffers passed in will be shrinked using
     * @c buffer_shrink().
     * @param idx Pointer to integer. This will be increased by 1 for
     *            every call (for easy iteration.)
     * @param name Where name of variable will be stored.
     * @param type Where type of variable will be stored.
     * @param init Where initial value of variable will be stored.
     * @param value Where value of variable will be stored.
     * @return Success, ie variable exists.
     */
    virtual bool getSingleOption (int* idx, buffer_t* name, buffer_t* type,
                                  buffer_t* init, buffer_t* value) = 0;
    /**
     * Init for a command.
     * @param ui User interface for error reporting.
     * @return Success.
     */
    virtual bool init (UI* ui) = 0;

    /** return values for all command handlers */
    enum state {
      /** okay, value not changed */
      S_OK_UNCHANGED = 0,
      /** okay, value changed */
      S_OK_CHANGED,
      /** command invalid (eg for options: @c toggle for string) */
      S_CMD,
      /** value invalid */
      S_VALUE
    };

};

#endif /* !MUTTNG_CONFIG_ABSTRACT_COMMAND_H */
