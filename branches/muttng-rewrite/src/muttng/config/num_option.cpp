/**
 * @file muttng/config/num_option.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Num variable implementation
 */
#include "core/buffer.h"
#include "core/mem.h"
#include "core/str.h"

#include "num_option.h"

NumOption::NumOption () {}
NumOption::~NumOption () {}

/** @bug atoi -> /dev/null */
AbstractOption::state NumOption::fromString (AbstractOption::commands command, buffer_t* src, option_t* dst) {
  int* ptr = (int*) dst->data;
  switch (command) {
    case T_SET: *ptr = atoi (src->str); break;
    case T_UNSET: *ptr = 0; break;
    case T_RESET: *ptr = atoi (dst->init); break;
    case T_TOGGLE:
    case T_QUERY:
      return (S_CMD);
  }
  return (S_OK);
}

void NumOption::toString (option_t* src, buffer_t* dst) {
  buffer_add_num (dst, *((int*) src->data), -1);
}
