/** @ingroup muttng_conf */
/**
 * @file muttng/config/string_option.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: String variable
 */
#include "core/buffer.h"
#include "core/mem.h"
#include "core/str.h"

#include "string_option.h"

StringOption::StringOption () {}
StringOption::~StringOption () {}

AbstractCommand::state StringOption::fromString (AbstractOption::commands command, buffer_t* src, option_t* dst) {
  char** str = (char**) dst->data;
  char* p = NULL;
  bool change = false;

  if (!src || !src->len)
    return (AbstractCommand::S_VALUE);

  switch (command) {
    case T_SET: p = src->str; break;
    case T_UNSET: break;
    case T_RESET: p = (char*) dst->init; break;
    case T_TOGGLE:
    case T_QUERY:
      return (AbstractCommand::S_CMD);
  }
  change = !str_eq ((str && *str ? *str : ""), p);
  str_replace (str, p);
  return (change ? AbstractCommand::S_OK_CHANGED : AbstractCommand::S_OK_UNCHANGED);
}

void StringOption::toString (option_t* src, buffer_t* dst) {
  buffer_add_str (dst, *((char**) src->data), -1);
}
