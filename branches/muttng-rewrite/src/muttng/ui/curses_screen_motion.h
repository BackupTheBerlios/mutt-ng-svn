/** @ingroup muttng_ui_curses */
/**
 * @file muttng/ui/curses_screen_motion.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Curses UI Screen superclass interface
 */
#ifndef MUTTNG_UI_CURSES_SCREEN_MOTION_H
#define MUTTNG_UI_CURSES_SCREEN_MOTION_H

#include "abstract_screen_motion.h"

/**
 * Dummy class with NOOP motions for CLI.
 */
class CursesScreenMotion : public AbstractScreenMotion {
  public:
    CursesScreenMotion (void);
    virtual ~CursesScreenMotion (void) = 0;
    virtual bool setFirst (void) = 0;
    virtual bool setLast (void) = 0;
};

#endif /* !MUTTNG_UI_CURSES_SCREEN_MOTION_H */
