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

bool StringOption::fromString (buffer_t* src, option_t* dst) {
  char** str = (char**) dst->data;
  str_replace (str, src->str);
  return (true);
}

bool StringOption::toString (option_t* src, buffer_t* dst) {
  buffer_add_str (dst, *((char**) src->data), -1);
  return (true);
}
