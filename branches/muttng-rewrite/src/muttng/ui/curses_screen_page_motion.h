/** @ingroup muttng_ui_curses */
/**
 * @file muttng/ui/curses_screen_page_motion.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Curses UI ScreenPage superclass interface
 */
#ifndef MUTTNG_UI_CURSES_SCREEN_PAGE_MOTION_H
#define MUTTNG_UI_CURSES_SCREEN_PAGE_MOTION_H

#include "curses_screen_motion.h"

/**
 * Page-based motion for Curses UI.
 */
class CursesScreenPageMotion : public CursesScreenMotion {
  public:
    CursesScreenPageMotion (void);
    ~CursesScreenPageMotion (void) = 0;
    bool setFirst (void);
    bool setLast (void);
};

#endif /* !MUTTNG_UI_CURSES_SCREEN_PAGE_MOTION_H */
