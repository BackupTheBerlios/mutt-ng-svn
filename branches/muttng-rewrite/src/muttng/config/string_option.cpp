/**
 * @file muttng/config/string_option.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief String variable implementation
 */
#include "core/buffer.h"
#include "core/mem.h"
#include "core/str.h"

#include "string_option.h"

StringOption::StringOption () {}
StringOption::~StringOption () {}

AbstractOption::state StringOption::fromString (AbstractOption::commands command, buffer_t* src, option_t* dst) {
  char** str = (char**) dst->data;
  char* p = NULL;

  switch (command) {
    case T_SET: p = src->str; break;
    case T_UNSET: break;
    case T_RESET: p = (char*) dst->init; break;
    case T_TOGGLE:
    case T_QUERY:
      return (S_CMD);
  }
  str_replace (str, p);
  return (S_OK);
}

void StringOption::toString (option_t* src, buffer_t* dst) {
  buffer_add_str (dst, *((char**) src->data), -1);
}
