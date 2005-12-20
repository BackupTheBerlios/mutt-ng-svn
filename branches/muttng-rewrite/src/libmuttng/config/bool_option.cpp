/** @addtogroup libmuttng_config */
/**
 * @file libmuttng/config/bool_option.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: Bool option
 */
#include <cstring>

#include "bool_option.h"

#include "core/intl.h"
#include "core/str.h"

BoolOption::BoolOption(const char* name_, const char* init_,
                       bool* store_) : Option(name_,init_), store(store_) {
  set(init_,NULL);
}

bool BoolOption::set(const char* value, buffer_t* error) {
  if (!value)
    return unset();

  bool old = *store;
  if (str_eq(value,"yes"))
    *store = true;
  else if (str_eq(value,"no"))
    *store = false;
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

bool BoolOption::unset() { *store = false; return true; }
bool BoolOption::reset() { set(init,NULL); return true; }
bool BoolOption::toggle() { *store = !*store; return true; }
bool BoolOption::query(buffer_t* dst) {
  if (!dst)
    return false;
  buffer_add_str(dst,*store?"yes":"no",*store?3:2);
  return true;
}

const char* BoolOption::getType() { return _("boolean"); }

bool BoolOption::validity(buffer_t* dst) { (void) dst; return false; }
