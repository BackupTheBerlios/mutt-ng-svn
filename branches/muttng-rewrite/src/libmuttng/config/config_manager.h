/**
 * @ingroup libmuttng
 * @addtogroup libmuttng_config Configuration Handling
 * @{
 */
/**
 * @file libmuttng/config/config_manager.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: Configuration Manager
 */
#ifndef LIBMUTTNG_CONFIG_CONFIG_MANAGER_H
#define LIBMUTTNG_CONFIG_CONFIG_MANAGER_H

#include "libmuttng/config/option.h"

#include "libmuttng/config/string_option.h"
#include "libmuttng/config/int_option.h"
#include "libmuttng/config/bool_option.h"
#include "libmuttng/config/url_option.h"
#include "libmuttng/config/rx_option.h"

#include "core/buffer.h"

#include <vector>

/**
 * Central configuration manager.
 */
class ConfigManager {
  private:
    ConfigManager();
    ~ConfigManager();
  public:
    /**
     * Register a new option.
     * @param option Option.
     * @return Success (i.e. didn't exist yet).
     */
    static bool reg(Option* option);
    /**
     * Set an option to a different value.
     * @param name Name of option.
     * @param value New value.
     * @param error Optional buffer for error message.
     * @return
     *  - @c true if the option exists and setting a new value succeeded
     *  - @c false otherwise
     */
    static bool set(const char* name, buffer_t* value, buffer_t* error);
    /**
     * Get a pointer to a single option.
     * @param name Name of the option to retrieve.
     * @return Option or @c NULL if not found.
     */
    static Option* get(const char* name);
    /** Initialize. */
    static bool init();
    /** Cleanup after use, ie when application exists */
    static bool cleanup();
    /**
     * Get list of all know options in any order.
     * @return List.
     */
    static std::vector<const char*>* getAll();
};

#endif /* !LIBMUTTNG_CONFIG_CONFIG_MANAGER_H */

/** @} */
