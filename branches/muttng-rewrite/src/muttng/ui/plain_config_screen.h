/** @ingroup muttng_ui_cli */
/**
 * @file muttng/ui/plain_config_screen.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Plain Config UI Screen superclass interface
 */
#ifndef MUTTNG_UI_PLAIN_CONFIG_SCREEN_H
#define MUTTNG_UI_PLAIN_CONFIG_SCREEN_H

#include "config_screen.h"
#include "plain_screen_motion.h"

/**
 * CLI configuration screen.
 */
class PlainConfigScreen : public PlainScreenMotion, public ConfigScreen {
  public:
    PlainConfigScreen (void);
    ~PlainConfigScreen (void);
    AbstractScreen::state getOp (void);
    void init (void);
    bool compile (const char* name, const char* value,
                  const char* type, const char* init);
};

#endif /* !MUTTNG_UI_PLAIN_CONFIG_SCREEN_H */
