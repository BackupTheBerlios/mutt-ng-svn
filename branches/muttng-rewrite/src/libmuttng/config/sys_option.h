/** @addtogroup libmuttng_config */
/**
 * @file libmuttng/config/sys_option.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: System option
 *
 * This file is published under the GNU General Public License.
 */
#ifndef LIBMUTTNG_CONFIG_SYS_OPTION_H
#define LIBMUTTNG_CONFIG_SYS_OPTION_H

#include "option.h"

/**
 * System option class
 */
class SysOption : public Option {
  public:
    /**
     * Create system boolean option.
     * @param name_ Name.
     * @param init_ Initial value.
     */
    SysOption(const char* name_, const char* init_);
    bool set(const char* value, buffer_t* error);
    bool unset();
    bool reset();
    bool toggle();
    bool query(buffer_t* dst);
    const char* getType();
    bool validity(buffer_t* dst);
};

#endif
