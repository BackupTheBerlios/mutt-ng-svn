/** @addtogroup libmuttng_config */
/**
 * @file libmuttng/config/option.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: Generic Config Option
 */
#ifndef LIBMUTTNG_CONFIG_OPTION_H
#define LIBMUTTNG_CONFIG_OPTION_H

#include "core/buffer.h"
#include "libmuttng/muttng_signal.h"

/**
 * Generic config option class.
 */
class Option {
  public:
    /**
     * Create new option.
     * Note that this class only maintains pointers but no copies of the
     * parameters as they're expected to be constant during runtime.
     * @param name_ Name.
     * @param init_ Initial value.
     */
    Option(const char* name_, const char* init_);
    virtual ~Option();
    /**
     * Set a new value for an option.
     * @param value String to extract value from.
     * @param error Optional buffer for where to write error message to.
     * @return Success.
     */
    virtual bool set(const char* value,buffer_t* error) = 0;
    /**
     * Set option to a neutral value.
     * @return Success.
     */
    virtual bool unset() = 0;
    /**
     * Reset option to initial value.
     * @return Success.
     */
    virtual bool reset() = 0;
    /**
     * Toggle or invert the current value.
     * @return @c true if supported, @c false otherwise.
     */
    virtual bool toggle() = 0;
    /**
     * Get the current value.
     * @param dst Destination buffer where to write to.
     * @return Success.
     */
    virtual bool query(buffer_t* dst) = 0;
    /**
     * Get validity information.
     * If no destination buffer is passed, just check if option
     * has limitations.
     */
    virtual bool validity(buffer_t* dst) = 0;
    /**
     * Signal emitted when the value changes.
     * The parameters are in order:
     *   -# pointer to option
     */
    Signal1<Option*> sigOptionChange;
    /**
     * Return name of option.
     * @return Name.
     */
    const char* getName();
    /**
     * Return initial value of option.
     * @return Value.
     */
    const char* getInit();
    /**
     * Get textual type of option.
     * @return Localized type string.
     */
    virtual const char* getType() = 0;
    /**
     * Escapes some special characters for printing config values.
     * @param dst Destination buffer.
     */
    static void prettyValue(buffer_t* dst);
  protected:
    /** the name */
    const char* name;
    /** the initial value */
    const char* init;
};

#endif
