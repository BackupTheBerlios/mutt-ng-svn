/** @ingroup muttng_conf */
/**
 * @file muttng/config/num_option.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: Numeric variable
 */
#include "core/buffer.h"
#include "core/mem.h"
#include "core/str.h"
#include "core/intl.h"

#include "num_option.h"

NumOption::NumOption () {}
NumOption::~NumOption () {}

AbstractCommand::state NumOption::fromString (AbstractOption::commands command,
                                              buffer_t* src, option_t* dst,
                                              buffer_t* error) {
  int* ptr = (int*) dst->data;
  int num = 0;
  size_t len = (error ? error->len : 0);
  const char* b = NULL;
  bool changed = false;

  if (!src || !src->len)
    return (AbstractCommand::S_VALUE);

  switch (command) {
    case T_SET: b = src->str; break;
    case T_UNSET: b = "0"; break;
    case T_RESET: b = dst->init; break;
    case T_TOGGLE:
    case T_QUERY:
      return (AbstractCommand::S_CMD);
  }
  if (!checkVal (b, dst, &num, error)) {
    if (error && error->len == len) {
      buffer_add_str (error, _("value out of range: "), -1);
      buffer_add_buffer (error, src);
    }
    return (AbstractCommand::S_VALUE);
  }
  changed = (*ptr != num);
  *ptr = num;
  return (changed ? AbstractCommand::S_OK_CHANGED :
                    AbstractCommand::S_OK_UNCHANGED);
}

void NumOption::toString (option_t* src, buffer_t* dst) {
  if (str_eq2 (src->name, "umask", 5))
    buffer_add_num2 (dst, *((int*) src->data), 4, 8);
  else
    buffer_add_num (dst, *((int*) src->data), -1);
}

bool NumOption::checkVal (const char* src, option_t* dst, int* num,
                          buffer_t* error) {
  char* e = NULL;

  if (!src || !*src)
    return (false);

  *num = strtol (src, &e, str_eq2 (dst->name, "umask", 5) ? 8 : 10);

  if (e && *e) {
    if (error) {
      buffer_add_str (error, _("not a number: "), -1);
      buffer_add_str (error, src, -1);
    }
    return (false);
  }

  if (str_eq2 (dst->name, "debug_level", 11))
    return (*num >= DEBUG_MIN && *num <= DEBUG_MAX);
  if (str_eq2 (dst->name, "umask", 5))
    return (*num >= 0 && *num <= 07777);
  return (true);
}
