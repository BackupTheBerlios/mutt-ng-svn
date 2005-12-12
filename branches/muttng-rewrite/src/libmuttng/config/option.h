#ifndef LIBMUTTNG_CONFIG_OPTION_H
#define LIBMUTTNG_CONFIG_OPTION_H

#include "core/buffer.h"
#include "libmuttng/muttng_signal.h"

class Option {
  public:
    Option(const char* name_, const char* init_);
    virtual ~Option();
    virtual bool set(const char* value,buffer_t* error) = 0;
    virtual bool unset() = 0;
    virtual bool reset() = 0;
    virtual bool toggle() = 0;
    virtual bool query(buffer_t* dst) = 0;
    Signal1<const char*> sigOptionChange;
    const char* getName();
    const char* getInit();
    virtual const char* getType() = 0;
  protected:
    const char* name;
    const char* init;
};

#endif
