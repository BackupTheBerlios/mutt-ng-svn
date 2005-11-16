/** @ingroup muttng_conf */
/**
 * @file muttng/config/string_option.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: String variable
 */
#ifndef MUTTNG_CONFIG_STRING_OPTION_H
#define MUTTNG_CONFIG_STRING_OPTION_H

#include "core/buffer.h"

#include "abstract_option.h"

/**
 * Class handling all string options.
 */
class StringOption : public AbstractOption {
  public:
    StringOption (void);
    ~StringOption (void);
    AbstractCommand::state fromString (AbstractOption::commands command,
                                      buffer_t* src, option_t* dst);
    void toString (option_t* src, buffer_t* dst);
};

#endif /* !MUTTNG_CONFIG_STRING_OPTION_H */
