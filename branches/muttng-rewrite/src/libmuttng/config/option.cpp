/** @addtogroup libmuttng_config */
/**
 * @file libmuttng/config/option.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: Generic Config Option
 */
#include "option.h"

Option::Option(const char* name_, const char* init_) : name(name_),init(init_) {}
Option::~Option(){}
const char* Option::getName() { return name; }
const char* Option::getInit() { return init; }

/** @bug move down to buffer.c */
void Option::prettyValue(buffer_t* dst) {
  if (!dst || !dst->len) return;
  buffer_t tmp;
  buffer_init(&tmp);
  char* p = dst->str;
  while (*p) {
    switch(*p) {
    case '\n': buffer_add_str(&tmp,"\\n",2); break;
    case '\r': buffer_add_str(&tmp,"\\r",2); break;
    case '\t': buffer_add_str(&tmp,"\\t",2); break;
    /*
     * since we strip one level of \ in config parser,
     * re-add here for copy'n'paste
     */
    case '\\': buffer_add_str(&tmp,"\\\\",2); break;
    default: buffer_add_ch(&tmp,*p); break;
    }
    p++;
  }
  buffer_shrink(dst,0);
  buffer_add_buffer(dst,&tmp);
  buffer_free(&tmp);
}
