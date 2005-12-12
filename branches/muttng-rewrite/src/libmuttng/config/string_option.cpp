#include "string_option.h"
#include "core/intl.h"
#include "core/str.h"
#include "core/mem.h"

StringOption::StringOption(const char* name_, const char* init_,
                     char** store_) : Option(name_,init_), store(store_) {
  set(init_,NULL);
}

bool StringOption::set(const char* value, buffer_t* error) {
  (void)error;
  if (!str_eq(*store,value)) {
    str_replace(store,value);
    sigOptionChange.emit(name);
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
