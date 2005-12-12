#ifndef LIBMUTTNG_CONFIG_INT_OPTION_H
#define LIBMUTTNG_CONFIG_INT_OPTION_H

#include "option.h"

class IntOption : public Option {
  public:
    IntOption(const char* name_, const char* init_, int* store_);
    bool set(const char* value, buffer_t* error);
    bool unset();
    bool reset();
    bool toggle();
    bool query(buffer_t* dst);
    const char* getType();
  private:
    int* store;
};

#endif
