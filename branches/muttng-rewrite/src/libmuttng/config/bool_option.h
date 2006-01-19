/** @addtogroup libmuttng_config */
/**
 * @file libmuttng/config/bool_option.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: Bool option
 *
 * This file is published under the GNU General Public License.
 */
#ifndef LIBMUTTNG_CONFIG_BOOL_OPTION_H
#define LIBMUTTNG_CONFIG_BOOL_OPTION_H

#include "option.h"

/**
 * Bool option class
 */
class BoolOption : public Option {
  public:
    /**
     * Create new boolean option.
     * @param name_ Name.
     * @param init_ Initial value.
     * @param store_ Storage for value.
     */
    BoolOption(const char* name_, const char* init_, bool* store_);
    bool set(const char* value, buffer_t* error);
    bool unset();
    bool reset();
    bool toggle();
    bool query(buffer_t* dst);
    const char* getType();
    bool validity(buffer_t* dst);
  private:
    /** storage for boolean. @todo optimize */
    bool* store;
};

#endif
