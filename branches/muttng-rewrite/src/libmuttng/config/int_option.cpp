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
                     int* store_, int min_, int max_) : 
  Option(name_,init_), store(store_), min(min_), max(max_) {
  set(init_,NULL);
}

IntOption::IntOption(const char* name_, const char* init_,
                     int* store_, bool negative) : 
  Option(name_,init_), store(store_) {
  if (negative) {
    min = INT_MIN;
    max = INT_MAX;
  } else {
    min = 0;
    max = INT_MAX;
  }
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

  if (num >= min && num <= max) {
    if (*store != num) {
      *store = num;
      sigOptionChange.emit(this);
    } else
      *store = num;
    return true;
  }
  /* value is invalid */
  if (error) {
    buffer_add_ch(error,'\'');
    buffer_add_snum(error,num,-1);
    buffer_add_str(error,_("' is invalid for $"),-1);
    buffer_add_str(error,name,-1);
  }
  return false;
}

bool IntOption::unset() {
  if (min == max)
    *store = 0;
  else
    *store = min;
  return true;
}
bool IntOption::reset() { set(init,NULL); return true; }
bool IntOption::toggle() { return false; }
bool IntOption::query(buffer_t* dst) {
  if (!dst)
    return false;
  buffer_add_snum(dst,*store,-1);
  return true;
}

const char* IntOption::getType() { return _("number"); }

bool IntOption::validity(buffer_t* dst) {
  if (!dst) return true;
  if (min == 0 && max == INT_MAX) {
    buffer_add_str(dst,_("positive"),-1);
  } else if (min != INT_MIN && max != INT_MAX) {  
    buffer_add_ch(dst,'[');
    buffer_add_snum(dst,min,-1);
    buffer_add_ch(dst,',');
    buffer_add_snum(dst,max,-1);
    buffer_add_ch(dst,']');
  } else
    return false;
  return true;
}
