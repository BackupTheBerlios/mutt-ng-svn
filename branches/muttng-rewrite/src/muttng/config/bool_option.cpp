/** @ingroup muttng_conf */
/**
 * @file muttng/config/bool_option.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: Bool variable
 */
#include "core/buffer.h"
#include "core/mem.h"
#include "core/str.h"
#include "core/alloca.h"

#include "bool_option.h"
#include "global_variables.h"

BoolOption::BoolOption () {}
BoolOption::~BoolOption () {}

AbstractOption::state BoolOption::fromString (AbstractOption::commands command,
                                              buffer_t* src, option_t* dst) {
  buffer_t* tmp;

  switch (command) {
    case T_UNSET:
      unset_option (dst->data);
      break;
    case T_TOGGLE:
      toggle_option (dst->data);
      break;
    case T_RESET:
    case T_SET:
      if (command == T_RESET) {
        tmp = (buffer_t*) alloca (sizeof (buffer_t));
        tmp->str = (char*) dst->init;
        tmp->len = str_len (tmp->str);
      } else
        tmp = src;
      if (buffer_equal1 (tmp, "yes", 3) || buffer_equal1 (tmp, "true", 4) ||
          buffer_equal1 (tmp, "1", 1))
        set_option (dst->data);
      else if (buffer_equal1 (tmp, "no", 2) || buffer_equal1 (tmp, "false", 5) ||
               buffer_equal1 (tmp, "0", 1))
        unset_option (dst->data);
      else
        return (S_VALUE);
      break;
    case T_QUERY:
      return (S_CMD);
  }
  return (S_OK);
}

void BoolOption::toString (option_t* src, buffer_t* dst) {
  if (option (src->data))
    buffer_add_str (dst, "yes", 3);
  else
    buffer_add_str (dst, "no", 2);
}
