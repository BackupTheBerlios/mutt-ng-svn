/** @ingroup muttng_conf */
/**
 * @file muttng/config/quad_option.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: Quad variable
 */
#include "core/buffer.h"
#include "core/mem.h"
#include "core/str.h"
#include "core/alloca.h"

#include "quad_option.h"
#include "global_variables.h"

QuadOption::QuadOption () {}
QuadOption::~QuadOption () {}

AbstractCommand::state QuadOption::fromString (AbstractOption::commands command,
                                               buffer_t* src, option_t* dst) {
  buffer_t* tmp;
  bool changed = false;

  switch (command) {
    case T_UNSET:
      if (src && src->len)
        return (AbstractCommand::S_VALUE);
      changed = setQuad (dst->data, Q_NO);
      break;
    case T_TOGGLE:
      if (src && src->len)
        return (AbstractCommand::S_VALUE);
      changed = true;
      toggleQuad (dst->data);
      break;
    case T_RESET:
    case T_SET:
      if (command == T_RESET) {
        tmp = (buffer_t*) alloca (sizeof (buffer_t));
        tmp->str = (char*) dst->init;
        tmp->len = str_len (tmp->str);
      } else
        tmp = src;
      if (!src || !src->len || buffer_equal1 (tmp, "yes", 3))
        changed = setQuad (dst->data, Q_YES);
      else if (src && src->len && buffer_equal1 (tmp, "ask-yes", 7))
        changed = setQuad (dst->data, Q_ASKYES);
      else if (src && src->len && buffer_equal1 (tmp, "no", 2))
        changed = setQuad (dst->data, Q_NO);
      else if (src && src->len && buffer_equal1 (tmp, "ask-no", 6))
        changed = setQuad (dst->data, Q_ASKNO);
      else
        return (AbstractCommand::S_VALUE);
      break;
    case T_QUERY:
      return (AbstractCommand::S_CMD);
  }
  return (changed ? AbstractCommand::S_OK_CHANGED :
                    AbstractCommand::S_OK_UNCHANGED);
}

void QuadOption::toString (option_t* src, buffer_t* dst) {
  switch (getQuad (src->data)) {
    case Q_NO: buffer_add_str (dst, "no", 2); break;
    case Q_YES: buffer_add_str (dst, "yes", 3); break;
    case Q_ASKNO: buffer_add_str (dst, "ask-no", 6); break;
    case Q_ASKYES: buffer_add_str (dst, "ask-yes", 7); break;
  }
}

bool QuadOption::setQuad (unsigned long idx, QuadOption::flags flag) {
  int i = (idx % Q_LAST), n = i/4, b = (i % 4) * 2;
  bool changed = getQuad (idx) != flag;
  QuadOptions[n] &= ~(0x3 << b);
  QuadOptions[n] |= (flag & 0x3) << b;
  return (changed);
}

void QuadOption::toggleQuad (unsigned long idx) {
  int i = (idx % Q_LAST), n = i/4, b = (i % 4) * 2;
  QuadOptions[n] ^= (1 << b);
}

QuadOption::flags QuadOption::getQuad (unsigned long idx) {
  int i = (idx % Q_LAST), n = i/4, b = (i % 4) * 2;
  return ((QuadOption::flags) ((QuadOptions[n] >> b) & 0x3));
}
