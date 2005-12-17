/** @addtogroup libmuttng_config */
/**
 * @file libmuttng/config/int_option.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: Integer option
 */
#ifndef LIBMUTTNG_CONFIG_INT_OPTION_H
#define LIBMUTTNG_CONFIG_INT_OPTION_H

#include "option.h"

#include <limits.h>

/**
 * Integer option class
 */
class IntOption : public Option {
  public:
    /**
     * Create new int option.
     * @param name_ Name.
     * @param init_ Initial value.
     * @param store_ Storage for value.
     * @param min_ Minimum value.
     * @param max_ Maximum value.
     */
    IntOption(const char* name_, const char* init_, int* store_,
              int min_ = INT_MIN, int max_ = INT_MAX);
    /**
     * Create new int option.
     * @param name_ Name.
     * @param init_ Initial value.
     * @param store_ Storage for value.
     * @param negative If @c true, negative values will be allowed.
     */
    IntOption(const char* name_, const char* init_, int* store_,
              bool negative);
    bool set(const char* value, buffer_t* error);
    bool unset();
    bool reset();
    bool toggle();
    bool query(buffer_t* dst);
    const char* getType();
    bool validity(buffer_t* dst);
  private:
    /** storage for value */
    int* store;
    /** for validation checks: minimum value allowed */
    int min;
    /** for validation checks: maximum value allowed */
    int max;
};

#endif
