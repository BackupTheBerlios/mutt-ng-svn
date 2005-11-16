/** @ingroup muttng_conf */
/**
 * @file muttng/config/bool_option.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: Bool variable
 */
#ifndef MUTTNG_CONFIG_BOOL_OPTION_H
#define MUTTNG_CONFIG_BOOL_OPTION_H

#include "core/buffer.h"

#include "abstract_option.h"

/**
 * Class handling all bool options.
 */
class BoolOption : public AbstractOption {
  public:
    BoolOption (void);
    ~BoolOption (void);
    AbstractCommand::state fromString (AbstractOption::commands command,
                                      buffer_t* src, option_t* dst);
    void toString (option_t* src, buffer_t* dst);
};

#endif /* !MUTTNG_CONFIG_BOOL_OPTION_H */
