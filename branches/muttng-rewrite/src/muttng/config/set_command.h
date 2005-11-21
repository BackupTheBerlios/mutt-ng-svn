/** @ingroup muttng_conf */
/**
 * @file muttng/config/set_command.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: @c set Command handler
 */
#ifndef MUTTNG_CONFIG_SET_COMMAND_H
#define MUTTNG_CONFIG_SET_COMMAND_H

#include "core/buffer.h"

#include "ui/ui.h"

#include "abstract_command.h"
#include "abstract_option.h"

/**
 * Class implementing the @c set, @c reset, @c unset and @c toggle
 * commands.
 */
class SetCommand : public AbstractCommand {
  public:
    /** constructor */
    SetCommand ();
    /** destructor */
    ~SetCommand ();
    /**
     * Handler.
     * @param line Configuration line.
     * @param error error Where to put error messages.
     * @param data Type of command received: SetCommand::commands.
     * @sa SetCommand::commands
     * @return Whether line is valid.
     */
    bool handle (buffer_t* line, buffer_t* error, unsigned long data);
    bool getSingleOption (int* idx, buffer_t* name, buffer_t* type,
                          buffer_t* init, buffer_t* value);
    /** Variable types we know */
    enum types {
      /** @c string */
      T_STRING = 0,
      /** @c boolean */
      T_BOOL,
      /** @c number */
      T_NUM,
      /** @c quad-option */
      T_QUAD,
      /** last */
      T_LAST
    };
    bool init (UI* ui);

  private:
    /** handlers for different option types */
    AbstractOption* handlers[T_LAST];
    /** string representation of types */
    const char* types[T_LAST];
};

#endif /* !MUTTNG_CONFIG_SET_COMMAND_H */
