#ifndef LIBMUTTNG_CONFIG_STRING_OPTION_H
#define LIBMUTTNG_CONFIG_STRING_OPTION_H

#include "option.h"

class StringOption : public Option {
  public:
    StringOption(const char* name_, const char* init_, char** store_);
    bool set(const char* value, buffer_t* error);
    bool unset();
    bool reset();
    bool toggle();
    bool query(buffer_t* dst);
    const char* getType();
  private:
    char** store;
};

#endif
