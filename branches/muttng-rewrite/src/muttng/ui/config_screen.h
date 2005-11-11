/** @ingroup muttng_ui */
/**
 * @file muttng/ui/config_screen.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Config UI Screen superclass interface
 */
#ifndef MUTTNG_UI_CONFIG_SCREEN_H
#define MUTTNG_UI_CONFIG_SCREEN_H

#include "abstract_screen.h"

/**
 * Configuration screen.
 */
class ConfigScreen : public AbstractScreen {
  public:
    /** constructor */
    ConfigScreen (void);
    /** destructor */
    virtual ~ConfigScreen (void) = 0;
    virtual AbstractScreen::state getOp (void) = 0;
    virtual void init (void) = 0;
    /**
     * Compile line.
     * @param name Variable name.
     * @param value Variable's value.
     * @param type Type string.
     * @param init Default value.
     * @return Success.
     */
    virtual bool compile (const char* name, const char* value,
                          const char* type, const char* init) = 0;
};

#endif /* !MUTTNG_UI_CONFIG_SCREEN_H */
