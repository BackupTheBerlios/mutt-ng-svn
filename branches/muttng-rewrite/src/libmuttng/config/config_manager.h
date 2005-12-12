#ifndef LIBMUTTNG_CONFIG_CONFIG_MANAGER_H
#define LIBMUTTNG_CONFIG_CONFIG_MANAGER_H

#include "option.h"
#include "string_option.h"
#include "int_option.h"

#include "core/buffer.h"

#include <vector>

class ConfigManager {
  private:
    ConfigManager();
    ~ConfigManager();
  public:
    static bool reg(Option* option);
    static bool set(const char* name, buffer_t* value, buffer_t* error);
    static Option* get(const char* name);
    static bool init();
    static bool cleanup();
    static std::vector<const char*>* getAll();
};

#endif /* !LIBMUTTNG_CONFIG_CONFIG_MANAGER_H */
