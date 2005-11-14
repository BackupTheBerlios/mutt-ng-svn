/**
 * @file muttng/config/num_option.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Num variable interface
 */
#ifndef MUTTNG_CONFIG_NUM_OPTION_H
#define MUTTNG_CONFIG_NUM_OPTION_H

#include "core/buffer.h"

#include "abstract_option.h"

/**
 * Class handling all numeric options.
 */
class NumOption : public AbstractOption {
  public:
    NumOption (void);
    ~NumOption (void);
    AbstractOption::state fromString (AbstractOption::commands command,
                                      buffer_t* src, option_t* dst);
    void toString (option_t* src, buffer_t* dst);
  private:
    /**
     * See if value would be valid for an option.
     * @param src Source input.
     * @param dst Option.
     * @param num Pointer to number storage.
     * @return Whether number is valid.
     */
    bool checkVal (const char* src, option_t* dst, int* num);
};

#endif /* !MUTTNG_CONFIG_NUM_OPTION_H */
