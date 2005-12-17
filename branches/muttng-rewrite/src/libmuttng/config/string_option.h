/** @addtogroup libmuttng_config */
/**
 * @file libmuttng/config/string_option.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: String option
 */
#ifndef LIBMUTTNG_CONFIG_STRING_OPTION_H
#define LIBMUTTNG_CONFIG_STRING_OPTION_H

#include "option.h"
#include "core/rx.h"

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
     * @param pattern_ Optional pattern to check value.
     */
    StringOption(const char* name_, const char* init_, char** store_,
                 const char* pattern_ = NULL);
    bool set(const char* value, buffer_t* error);
    bool unset();
    bool reset();
    bool toggle();
    bool query(buffer_t* dst);
    const char* getType();
    bool validity(buffer_t* dst);
  private:
    /** optional pattern for validation */
    rx_t* pattern;
    /** storage for value */
    char** store;
};

#endif
