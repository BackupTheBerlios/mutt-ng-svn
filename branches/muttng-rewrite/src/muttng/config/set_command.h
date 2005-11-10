/**
 * @file muttng/config/set_command.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief @c set Command handler interface
 */
#ifndef MUTTNG_CONFIG_SET_COMMAND_H
#define MUTTNG_CONFIG_SET_COMMAND_H

#include "core/buffer.h"

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
    bool print (buffer_t* dst, bool changedOnly = false,
                bool annotated = false);
    /** Configuration commands we're reponsible for. */
    enum commands {
      /** @c set keyword */
      T_SET = 0,
      /** @c unset keyword */
      T_UNSET,
      /** @c reset keyword */
      T_RESET,
      /** @c toggle keyword */
      T_TOGGLE
    };
    /** Variable types we know */
    enum types {
      /** @c string */
      T_STRING = 0,
#if 0
      /** @c boolean */
      T_BOOL,
      /** @c number */
      T_NUM,
#endif
      /** last */
      T_LAST
    };
    void init (void);
  private:
    AbstractOption* handlers[T_LAST];
};

#endif /* !MUTTNG_CONFIG_SET_COMMAND_H */
