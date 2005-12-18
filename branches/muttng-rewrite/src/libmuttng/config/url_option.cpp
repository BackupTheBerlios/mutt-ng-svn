/** @addtogroup libmuttng_config */
/**
 * @file libmuttng/config/url_option.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: URL option
 */
#include "url_option.h"
#include "core/intl.h"

#include <iostream>

URLOption::URLOption(const char* name_, const char* init_,
                     url_t** store_) : Option(name_,init_), store(store_) {
  set(init_,NULL);
}

URLOption::~URLOption() {
  url_free(*store);
}

bool URLOption::set(const char* value, buffer_t* error) {
  url_t* u = NULL;
  if (!(u = url_from_string(value,error)))
    return false;
  bool change = !url_eq(u,*store);
  url_free(*store);
  delete *store;
  *store = u;
  if (change)
    sigOptionChange.emit(this);
  return true;
}

bool URLOption::unset() { return false; }
bool URLOption::reset() { set(init,NULL); return true; }
bool URLOption::toggle() { return false; }
bool URLOption::query(buffer_t* dst) {
  if (!dst)
    return false;
  url_to_string(*store,dst,false);
  return true;
}

const char* URLOption::getType() { return _("URL"); }

bool URLOption::validity(buffer_t* dst) {
  url_syntax(dst);
  return true;
}
