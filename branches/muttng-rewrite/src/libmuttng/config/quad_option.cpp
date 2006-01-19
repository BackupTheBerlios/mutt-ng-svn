/** @addtogroup libmuttng_config */
/**
 * @file libmuttng/config/quad_option.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: Quad option
 *
 * This file is published under the GNU General Public License.
 */
#include <cstring>
#include <string.h>

#include "quad_option.h"

#include "core/intl.h"
#include "core/str.h"

QuadOption::QuadOption(const char* name_, const char* init_,
                       unsigned short* store_) : Option(name_,init_), store(store_) {
  set(init,NULL);
}

bool QuadOption::set(const char* value, buffer_t* error) {
  if (!value)
    return unset();

  unsigned short old = *store;
  if (str_eq(value,"yes"))
    *store = M_YES;
  else if (str_eq(value,"no"))
    *store = M_NO;
  else if (str_eq(value,"ask-yes"))
    *store = M_ASK_YES;
  else if (str_eq(value,"ask-no"))
    *store = M_ASK_NO;
  else {
    if (error) {
      buffer_add_ch(error,'\'');
      buffer_add_str(error,value,-1);
      buffer_add_str(error,_("' is invalid for $"),-1);
      buffer_add_str(error,name,-1);
    }
    return false;
  }
  if (old != *store)
    sigOptionChange.emit(this);
  return true;
}

bool QuadOption::unset() { *store = M_NO; return true; }
bool QuadOption::reset() { set(init,NULL); return true; }

bool QuadOption::toggle() {
  switch ((QuadOption::state)*store) {
  case M_NO: *store = M_YES; break;
  case M_YES: *store = M_NO; break;
  case M_ASK_YES: *store = M_ASK_NO; break;
  case M_ASK_NO: *store = M_ASK_YES; break;
  }
  return true;
}

bool QuadOption::query(buffer_t* dst) {
  if (!dst) return false;
  switch ((QuadOption::state)*store) {
  case M_NO: buffer_add_str(dst,"yes",3); break;
  case M_YES: buffer_add_str(dst,"no",2); break;
  case M_ASK_YES: buffer_add_str(dst,"ask-yes",7); break;
  case M_ASK_NO: buffer_add_str(dst,"ask-no",6); break;
  }
  return true;
}

const char* QuadOption::getType() { return _("quad-option"); }

bool QuadOption::validity(buffer_t* dst) { (void) dst; return false; }
