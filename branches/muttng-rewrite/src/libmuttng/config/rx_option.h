/** @addtogroup libmuttng_config */
/**
 * @file libmuttng/config/rx_option.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Boolerface: Regex option
 */
#ifndef LIBMUTTNG_CONFIG_REGEX_OPTION_H
#define LIBMUTTNG_CONFIG_REGEX_OPTION_H

#include "option.h"
#include "core/rx.h"

/**
 * Regex option class
 */
class RXOption : public Option {
  public:
    /**
     * Create new boolean option.
     * @param name_ Name.
     * @param init_ Initial value.
     * @param store_ Storage for value.
     */
    RXOption(const char* name_, const char* init_, rx_t** store_);
    bool set(const char* value, buffer_t* error);
    bool unset();
    bool reset();
    bool toggle();
    bool query(buffer_t* dst);
    const char* getType();
    bool validity(buffer_t* dst);
  private:
    rx_t** store;
};

#endif
