/** @ingroup muttng_conf */
/**
 * @file muttng/config/url_option.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: URL variable
 */
#include "core/buffer.h"
#include "core/mem.h"
#include "core/str.h"

#include "libmuttng/util/url.h"

#include "url_option.h"

URLOption::URLOption () {}
URLOption::~URLOption () {}

AbstractCommand::state URLOption::fromString (AbstractOption::commands command,
                                              buffer_t* src, option_t* dst,
                                              buffer_t* error) {
  char* p = NULL;
  bool change = false;
  url_t* url = NULL;
  url_t* old = (url_t*) dst->data;

  switch (command) {
    case T_UNSET:
      change = old != NULL;
      delete (old);
      old = NULL;
      break;
    case T_SET:
    case T_RESET:
      p = (char*) (command == T_SET ? src->str : dst->init);
      if ((url = url_from_string (p, error)) || !src || !src->len)
        dst->data = (unsigned long) url;
      else
        return (AbstractCommand::S_VALUE);
      break;
    case T_TOGGLE:
    case T_QUERY:
      return (AbstractCommand::S_CMD);
  }
  return (change ? AbstractCommand::S_OK_CHANGED :
                   AbstractCommand::S_OK_UNCHANGED);
}

void URLOption::toString (option_t* src, buffer_t* dst) {
  url_to_string ((url_t*) src->data, dst, false);
}
