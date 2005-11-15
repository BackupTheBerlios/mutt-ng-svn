/** @ingroup muttng_conf */
/**
 * @file muttng/config/num_option.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: Numeric variable
 */
#include "core/buffer.h"
#include "core/mem.h"
#include "core/str.h"

#include "num_option.h"

NumOption::NumOption () {}
NumOption::~NumOption () {}

AbstractOption::state NumOption::fromString (AbstractOption::commands command, buffer_t* src, option_t* dst) {
  int* ptr = (int*) dst->data;
  int num = 0;
  const char* b = NULL;

  switch (command) {
    case T_SET: b = src->str; break;
    case T_UNSET: b = "0"; break;
    case T_RESET: b = dst->init; break;
    case T_TOGGLE:
    case T_QUERY:
      return (S_CMD);
  }
  if (!checkVal (b, dst, &num))
    return (S_VALUE);
  *ptr = num;
  return (S_OK);
}

void NumOption::toString (option_t* src, buffer_t* dst) {
  if (str_eq2 (src->name, "umask", 5))
    buffer_add_num2 (dst, *((int*) src->data), 4, 8);
  else
    buffer_add_num (dst, *((int*) src->data), -1);
}

bool NumOption::checkVal (const char* src, option_t* dst, int* num) {
  char* error = NULL;

  *num = strtol (src, &error, str_eq2 (dst->name, "umask", 5) ? 8 : 10);

  if (error && *error)
    return (false);

  if (str_eq2 (dst->name, "debug_level", 11))
    return (*num >= DEBUG_MIN && *num <= DEBUG_MAX);
  if (str_eq2 (dst->name, "umask", 5))
    return (*num >= 0 && *num <= 07777);
  return (true);
}
