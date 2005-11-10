/**
 * @file muttng/config/abstract_option.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Configuration variable superclass interface
 */
#ifndef MUTTNG_CONFIG_ABSTRACT_OPTION_H
#define MUTTNG_CONFIG_ABSTRACT_OPTION_H

#include "core/buffer.h"

#include "option.h"

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
class AbstractOption {
  public:
    AbstractOption (void);
    virtual ~AbstractOption (void) = 0;
    /**
     * Read option from string, to be overwritten by derived classes.
     * @param src Configuration line with value.
     * @param dst Destination storage.
     * @return Whether operation succeeded.
     */
    virtual bool fromString (buffer_t* src, option_t* dst) = 0;
    /**
     * Write option to string, to be overwritten by derived classes.
     * @param src Option.
     * @param dst Destination buffer.
     * @return Whether operation succeeded.
     */
    virtual bool toString (option_t* src, buffer_t* dst) = 0;
};

#endif /* !MUTTNG_CONFIG_ABSTRACT_OPTION_H */
