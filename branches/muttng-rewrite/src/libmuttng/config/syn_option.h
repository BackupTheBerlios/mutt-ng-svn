/** @addtogroup libmuttng_config */
/**
 * @file libmuttng/config/syn_option.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: Synonym option
 *
 * This file is published under the GNU General Public License.
 */
#ifndef LIBMUTTNG_CONFIG_SYN_OPTION_H
#define LIBMUTTNG_CONFIG_SYN_OPTION_H

#include "option.h"

/**
 * Synonym option class
 */
class SynOption : public Option {
  public:
    /**
     * Create new synonym option.
     * @param name_ Name of the synonym.
     * @param real_ What the synonym resolves to.
     */
    SynOption(const char* name_, const char* real_);
    /**
     * Create new synonym option.
     * @param name_ Name of the synonym.
     * @param real_ What the synonym resolves to.
     */
    SynOption(const char* name_, Option* real_);
    bool set(const char* value, buffer_t* error);
    bool unset();
    bool reset();
    bool toggle();
    bool query(buffer_t* dst);
    const char* getType();
    bool validity(buffer_t* dst);
  private:
    /** pointer to real option */
    Option* real;
};

#endif
