/** @ingroup muttng_conf */
/**
 * @file muttng/config/quad_option.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: Quad variable
 */
#ifndef MUTTNG_CONFIG_QUAD_OPTION_H
#define MUTTNG_CONFIG_QUAD_OPTION_H

#include "core/buffer.h"

#include "abstract_option.h"

/**
 * Class handling all quad options.
 */
class QuadOption : public AbstractOption {
  public:
    QuadOption (void);
    ~QuadOption (void);
    AbstractCommand::state fromString (AbstractOption::commands command,
                                      buffer_t* src, option_t* dst,
                                      buffer_t* error);
    void toString (option_t* src, buffer_t* dst);
  private:
    /** possible values */
    enum flags {
      /** @c no */
      Q_NO = 0,
      /** @c yes */
      Q_YES,
      /** @c ask-no */
      Q_ASKNO,
      /** @c ask-yes */
      Q_ASKYES
    };
    /**
     * Attempt to change a quad-option.
     * @param idx Option's index into QuadOptions.
     * @param flag To what value to change.
     * @return Whether the value for the option changed
     */
    bool setQuad (unsigned long idx, QuadOption::flags flag);
    /**
     * Toggle a value.
     * @param idx Option's index into QuadOptions.
     */
    void toggleQuad (unsigned long idx);
    /**
     * Get value of an option.
     * @param idx Option's index into QuadOptions.
     * @return Value.
     */
    QuadOption::flags getQuad (unsigned long idx);
};

#endif /* !MUTTNG_CONFIG_QUAD_OPTION_H */
