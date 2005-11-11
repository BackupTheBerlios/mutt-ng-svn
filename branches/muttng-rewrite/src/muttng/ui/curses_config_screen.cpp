/** @ingroup muttng_ui_curses */
/**
 * @file muttng/ui/curses_config_screen.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Curses Config UI Screen superclass interface
 */
#include "curses_config_screen.h"

CursesConfigScreen::CursesConfigScreen (void) {}
CursesConfigScreen::~CursesConfigScreen (void) {}

void CursesConfigScreen::init (void) {
}

bool CursesConfigScreen::compile (const char* name, const char* value,
                                  const char* type, const char* init) {
  (void) name;
  (void) value;
  (void) type;
  (void) init;
  return (true);
}

AbstractScreen::state CursesConfigScreen::getOp (void) {
  return (AbstractScreen::T_KNOWN_OK);
}
