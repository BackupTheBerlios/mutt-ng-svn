/** @addtogroup libmuttng_config */
/**
 * @file libmuttng/config/string_option.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: String option
 */
#ifndef LIBMUTTNG_CONFIG_STRING_OPTION_H
#define LIBMUTTNG_CONFIG_STRING_OPTION_H

#include "option.h"

/**
 * String option class.
 */
class StringOption : public Option {
  public:
    /**
     * Create string int option.
     * @param name_ Name.
     * @param init_ Initial value.
     * @param store_ Storage for value.
     */
    StringOption(const char* name_, const char* init_, char** store_);
    bool set(const char* value, buffer_t* error);
    bool unset();
    bool reset();
    bool toggle();
    bool query(buffer_t* dst);
    const char* getType();
  private:
    char** store;
};

#endif
