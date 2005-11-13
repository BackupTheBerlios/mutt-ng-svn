/**
 * @file muttng/config/config.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Configuration interface
 */
#ifndef MUTTNG_MUTTNG_CONFIG_H
#define MUTTNG_MUTTNG_CONFIG_H

#include "core/buffer.h"

#include "muttng.h"
#include "ui/ui.h"
#include "set_command.h"

/**
 * Configuration handling class.
 * @bug I'd love this static only but we get in trouble with pointer and
 * class tables here... ;-((
 */
class Config : public Muttng {
  public:
    /** constructor */
    Config (void);
    /** destructor */
    ~Config (void);
    /**
     * Read configuration from files.
     * @param readGlobal Whether to read system-wide config file.
     * @param file Configuration file different from user's default.
     * @param error Where to put error messages.
     * @return Success.
     */
    bool read (bool readGlobal = true, const char* file = NULL,
               buffer_t* error = NULL);
    /** Configuration commands we have handler classes for. */
    enum commands {
      /** @c set, @c unset, @c reset and  @c toggle. */
      C_SET = 0,
      /** last */
      C_INVALID
    };
    /**
     * Initialize config prior to reading config files.
     * @param ui User interface for error reporting.
     * @return Success.
     */
    bool init (UI* ui);

    bool getSingleOption (int* idx, buffer_t* name, buffer_t* type,
                          buffer_t* init, buffer_t* value);

  private:
    /** Table mapping Config::commands to AbstractCommand classes. */
    AbstractCommand* handlers[C_INVALID];
};

#endif /* !MUTTNG_MUTTNG_CONFIG_H */
