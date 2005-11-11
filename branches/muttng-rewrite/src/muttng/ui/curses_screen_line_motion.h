/** @ingroup muttng_ui_curses */
/**
 * @file muttng/ui/curses_screen_line_motion.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Curses UI ScreenLine superclass interface
 */
#ifndef MUTTNG_UI_CURSES_SCREEN_LINE_MOTION_H
#define MUTTNG_UI_CURSES_SCREEN_LINE_MOTION_H

#include "curses_screen_motion.h"

/**
 * Line-based motion for Curses UI.
 */
class CursesScreenLineMotion : public CursesScreenMotion {
  public:
    CursesScreenLineMotion (void);
    ~CursesScreenLineMotion (void) = 0;
    bool setFirst (void);
    bool setLast (void);
};

#endif /* !MUTTNG_UI_CURSES_SCREEN_LINE_MOTION_H */
