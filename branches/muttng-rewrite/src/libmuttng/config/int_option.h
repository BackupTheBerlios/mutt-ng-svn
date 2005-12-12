/** @addtogroup libmuttng_config */
/**
 * @file libmuttng/config/int_option.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: Integer option
 */
#ifndef LIBMUTTNG_CONFIG_INT_OPTION_H
#define LIBMUTTNG_CONFIG_INT_OPTION_H

#include "option.h"

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
     */
    IntOption(const char* name_, const char* init_, int* store_);
    bool set(const char* value, buffer_t* error);
    bool unset();
    bool reset();
    bool toggle();
    bool query(buffer_t* dst);
    const char* getType();
  private:
    int* store;
};

#endif
