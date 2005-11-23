/** @ingroup muttng_conf */
/**
 * @file muttng/config/url_option.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: URL variable
 */
#ifndef MUTTNG_CONFIG_URL_OPTION_H
#define MUTTNG_CONFIG_URL_OPTION_H

#include "core/buffer.h"

#include "abstract_option.h"

/**
 * Class handling all url options.
 */
class URLOption : public AbstractOption {
  public:
    URLOption (void);
    ~URLOption (void);
    AbstractCommand::state fromString (AbstractOption::commands command,
                                      buffer_t* src, option_t* dst,
                                      buffer_t* error);
    void toString (option_t* src, buffer_t* dst);
};

#endif /* !MUTTNG_CONFIG_URL_OPTION_H */
