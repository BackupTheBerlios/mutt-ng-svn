/** @ingroup muttng_ui_curses */
/**
 * @file muttng/ui/curses_screen_page_motion.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Curses UI ScreenPage superclass interface
 */
#include "curses_screen_page_motion.h"

CursesScreenPageMotion::CursesScreenPageMotion (void) {}
CursesScreenPageMotion::~CursesScreenPageMotion (void) {}

bool CursesScreenPageMotion::setFirst (void) { return (true); }
bool CursesScreenPageMotion::setLast (void) { return (true); }
