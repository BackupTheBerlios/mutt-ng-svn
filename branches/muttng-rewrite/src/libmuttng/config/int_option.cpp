/** @addtogroup libmuttng_config */
/**
 * @file libmuttng/config/int_option.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: Integer option
 */
#include "int_option.h"
#include "core/intl.h"

#include <iostream>

IntOption::IntOption(const char* name_, const char* init_,
                     int* store_) : Option(name_,init_), store(store_) {
  set(init_,NULL);
}

bool IntOption::set(const char* value, buffer_t* error) {
  char* e;

  if (!value)
    return unset();

  int num = strtol(value,&e,10);
  if (e && *e) {
    if (error) {
      buffer_add_str(error,_("not a number: '"),-1);
      buffer_add_str(error,value,-1);
      buffer_add_ch(error,'\'');
    }
    return false;
  }


  if (*store != num) {
    *store = num;
    sigOptionChange.emit(name);
  } else
    *store = num;
  return true;
}

bool IntOption::unset() { *store = 0; return true; }
bool IntOption::reset() { set(init,NULL); return true; }
bool IntOption::toggle() { return false; }
bool IntOption::query(buffer_t* dst) {
  if (!dst)
    return false;
  buffer_add_num(dst,*store,-1);
  return true;
}

const char* IntOption::getType() { return _("number"); }
