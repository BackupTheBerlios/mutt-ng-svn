/** @addtogroup libmuttng_config */
/**
 * @file libmuttng/config/rx_option.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: Regex option
 *
 * This file is published under the GNU General Public License.
 */
#include "rx_option.h"
#include "core/intl.h"
#include "core/mem.h"
#include "core/str.h"

#include <iostream>

RXOption::RXOption(const char* name_, const char* init_,
                   rx_t** store_) : Option(name_,init_), store(store_) {
  set(init,NULL);
}

RXOption::~RXOption() {
  rx_free(*store);
  mem_free(store);
}

bool RXOption::set(const char* value, buffer_t* error) {
  rx_t* r = NULL;
  if (!(r = rx_compile(value,error,0,0)))
    return false;
  bool change = !rx_eq(r,*store);
  rx_free(*store);
  mem_free(store);
  *store = r;
  if (change)
    sigOptionChange.emit(this);
  return true;
}

bool RXOption::unset() { return false; }
bool RXOption::reset() { set(init,NULL); return true; }
bool RXOption::toggle() { return false; }
bool RXOption::query(buffer_t* dst) {
  if (!dst)
    return false;
  buffer_add_str(dst,(*store)->pattern,-1);
  return true;
}

const char* RXOption::getType() { return _("regular expression"); }

bool RXOption::validity(buffer_t* dst) { (void) dst; return false; }
