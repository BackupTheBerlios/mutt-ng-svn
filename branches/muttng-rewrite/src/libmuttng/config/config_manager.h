/**
 * @ingroup libmuttng
 * @addtogroup libmuttng_config Configuration Handling
 * @{
 */
/**
 * @file libmuttng/config/config_manager.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: Configuration Manager
 *
 * This file is published under the GNU General Public License.
 */
#ifndef LIBMUTTNG_CONFIG_CONFIG_MANAGER_H
#define LIBMUTTNG_CONFIG_CONFIG_MANAGER_H

#include "libmuttng/config/option.h"

#include "libmuttng/config/string_option.h"
#include "libmuttng/config/int_option.h"
#include "libmuttng/config/bool_option.h"
#include "libmuttng/config/url_option.h"
#include "libmuttng/config/rx_option.h"
#include "libmuttng/config/quad_option.h"
#include "libmuttng/config/sys_option.h"
#include "libmuttng/config/syn_option.h"

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
     * @return The option if added and @c NULL if it existed already.
     */
    static Option* regOption(Option* option);
    /**
     * Register a new feature.
     * @param name Name of feature.
     */
    static void regFeature(const char* name);
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
    /**
     * Get the value for an option.
     * This can be used as callback for buffer_extract_token().
     * @param dst Destination buffer.
     * @param name Name of variable.
     * @return 1.
     */
    static int get(buffer_t* dst, buffer_t* name);
    /** Initialize. */
    static bool init();
    /** Cleanup after use, ie when application exists */
    static bool cleanup();
    /**
     * Get list of all know options in sorted order.
     * @return List.
     */
    static std::vector<const char*>* getOptions();
    /**
     * Get list of all known features in sorted order
     * @return List.
     */
    static std::vector<const char*>* getFeatures();
};

#endif /* !LIBMUTTNG_CONFIG_CONFIG_MANAGER_H */

/** @} */
