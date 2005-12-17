/** @addtogroup libmuttng_config */
/**
 * @file libmuttng/config/url_option.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Boolerface: URL option
 */
#ifndef LIBMUTTNG_CONFIG_URL_OPTION_H
#define LIBMUTTNG_CONFIG_URL_OPTION_H

#include "libmuttng/config/option.h"
#include "libmuttng/util/url.h"

/**
 * URL option class
 */
class URLOption : public Option {
  public:
    /**
     * Create new boolean option.
     * @param name_ Name.
     * @param init_ Initial value.
     * @param store_ Storage for value.
     */
    URLOption(const char* name_, const char* init_, url_t** store_);
    bool set(const char* value, buffer_t* error);
    bool unset();
    bool reset();
    bool toggle();
    bool query(buffer_t* dst);
    const char* getType();
    bool validity(buffer_t* dst);
  private:
    /** storage for value */
    url_t** store;
};

#endif
