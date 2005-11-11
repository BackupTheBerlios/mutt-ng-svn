/** @ingroup muttng_ui_curses */
/**
 * @file muttng/ui/curses_screen_line_motion.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Curses UI ScreenLine superclass interface
 */
#include "curses_screen_line_motion.h"

CursesScreenLineMotion::CursesScreenLineMotion (void) {}
CursesScreenLineMotion::~CursesScreenLineMotion (void) {}

bool CursesScreenLineMotion::setFirst (void) { return (true); }
bool CursesScreenLineMotion::setLast (void) { return (true); }
