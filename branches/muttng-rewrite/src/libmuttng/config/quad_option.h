/** @addtogroup libmuttng_config */
/**
 * @file libmuttng/config/quad_option.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Boolerface: Quad option
 *
 * This file is published under the GNU General Public License.
 */
#ifndef LIBMUTTNG_CONFIG_QUAD_OPTION_H
#define LIBMUTTNG_CONFIG_QUAD_OPTION_H

#include "option.h"

/**
 * Quad option class
 */
class QuadOption : public Option {
  public:
    /**
     * Create new boolean option.
     * @param name_ Name.
     * @param init_ Initial value.
     * @param store_ Storage for value.
     */
    QuadOption(const char* name_, const char* init_, unsigned short* store_);
    bool set(const char* value, buffer_t* error);
    bool unset();
    bool reset();
    bool toggle();
    bool query(buffer_t* dst);
    const char* getType();
    bool validity(buffer_t* dst);
  private:
    /** quad-option states */
    enum state { M_YES = 0, M_NO, M_ASK_YES, M_ASK_NO };
    /** storage for value */
    unsigned short* store;
};

#endif
