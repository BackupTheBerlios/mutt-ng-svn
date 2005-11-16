/** @ingroup muttng_conf */
/**
 * @file muttng/config/abstract_option.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: Configuration variable base class
 */
#ifndef MUTTNG_CONFIG_ABSTRACT_OPTION_H
#define MUTTNG_CONFIG_ABSTRACT_OPTION_H

#include "core/buffer.h"

#include "muttng.h"
#include "option.h"
#include "abstract_command.h"

/**
 * Abstract class for configuration variable handlers.
 *
 * As conversion and reading variables works from/to strings only,
 * implementing classes only need need to provide a function reading
 * from string and one writing to it.
 *
 * These calls may fail whereby it depends on the module what exactly
 * failure means and why it failed. For example: for strings we may have
 * options allowing particular values only so that an attempt to set
 * invalid magic word may fail. Or a negative number supposed to replace
 * some positive integer only.
 *
 * Also, when the event handling framework is done, both may also raise
 * events as they'd like to (e.g. redraw a menu when a layout-specific
 * variable was changed.)
 */
class AbstractOption : public Muttng {
  public:
    AbstractOption (void);
    virtual ~AbstractOption (void) = 0;
    /** all configuration commands known for config variables */
    enum commands {
      /** @c set */
      T_SET = 0,
      /** @c unset */
      T_UNSET,
      /** @c reset */
      T_RESET,
      /** @c toggle */
      T_TOGGLE,
      /** @c query */
      T_QUERY
    };
    /**
     * Read option from string, to be overwritten by derived classes.
     * @param command For which config command to read value.
     * @param src Configuration line with value.
     * @param dst Destination storage.
     * @return Whether operation succeeded.
     */
    virtual AbstractCommand::state fromString (AbstractOption::commands command,
                                              buffer_t* src, option_t* dst) = 0;
    /**
     * Write option to string, to be overwritten by derived classes.
     * @param src Option.
     * @param dst Destination buffer.
     */
    virtual void toString (option_t* src, buffer_t* dst) = 0;
};

#endif /* !MUTTNG_CONFIG_ABSTRACT_OPTION_H */
