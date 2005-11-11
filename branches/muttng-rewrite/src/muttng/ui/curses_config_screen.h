/** @ingroup muttng_ui_curses */
/**
 * @file muttng/ui/curses_config_screen.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Curses Config UI Screen superclass interface
 */
#ifndef MUTTNG_UI_CURSES_CONFIG_SCREEN_H
#define MUTTNG_UI_CURSES_CONFIG_SCREEN_H

#include "config_screen.h"
#include "curses_screen_page_motion.h"

/**
 * Curses configuration screen.
 */
class CursesConfigScreen : public CursesScreenPageMotion, public ConfigScreen {
  public:
    CursesConfigScreen (void);
    ~CursesConfigScreen (void);
    AbstractScreen::state getOp (void);
    void init (void);
    bool compile (const char* name, const char* value,
                  const char* type, const char* init);
};

#endif /* !MUTTNG_UI_CURSES_CONFIG_SCREEN_H */
