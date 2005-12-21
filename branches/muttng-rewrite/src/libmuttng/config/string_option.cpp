/** @addtogroup libmuttng_config */
/**
 * @file libmuttng/config/string_option.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: String option
 */
#include <cstring>
#include <string.h>

#include "string_option.h"
#include "core/intl.h"
#include "core/str.h"
#include "core/mem.h"

StringOption::StringOption(const char* name_, const char* init_,
                           char** store_, const char* pattern_) : Option(name_,init_), store(store_) {
  if (pattern_)
    pattern = rx_compile(pattern_,NULL,0);
  else
    pattern = NULL;
  set(init,NULL);
}

StringOption::~StringOption() {
  mem_free(store);
  rx_free(pattern);
  mem_free(&pattern);
}

bool StringOption::set(const char* value, buffer_t* error) {
  if (pattern && !rx_exec(pattern,value)) {
    if (error) {
      buffer_add_ch(error,'\'');
      buffer_add_str(error,value,-1);
      buffer_add_str(error,_("' is invalid for $"),-1);
      buffer_add_str(error,name,-1);
    }
    return false;
  }
  if (!str_eq(*store,value)) {
    str_replace(store,value);
    sigOptionChange.emit(this);
  } else
    str_replace(store,value);
  return true;
}

bool StringOption::unset() { mem_free(store); return true; }
bool StringOption::reset() { set(init,NULL); return true; }
bool StringOption::toggle() { return false; }
bool StringOption::query(buffer_t* dst) {
  if (!dst)
    return false;
  buffer_add_str(dst,*store,-1);
  return true;
}

const char* StringOption::getType() { return _("string"); }

bool StringOption::validity(buffer_t* dst) {
  if (!dst || !pattern) return pattern!=NULL;
  buffer_add_str(dst,pattern->pattern,-1);
  return true;
}
