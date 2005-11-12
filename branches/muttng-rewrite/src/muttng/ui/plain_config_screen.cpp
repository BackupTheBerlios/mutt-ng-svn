/** @ingroup muttng_ui_cli */
/**
 * @file muttng/ui/plain_config_screen.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Plain Config UI Screen superclass implementation
 */
#include <iostream>

#include "plain_config_screen.h"
#include "abstract_screen.h"

using namespace std;

PlainConfigScreen::PlainConfigScreen (void) {}
PlainConfigScreen::~PlainConfigScreen (void) {}
AbstractScreen::state PlainConfigScreen::getOp (void) {
  return (AbstractScreen::T_KNOWN_OK);
}
void PlainConfigScreen::init (void) {}
bool PlainConfigScreen::compile (const char* name,
                                 const char* value,
                                 const char* type, const char* init) {
  cout << name << " = \"" << value << "\"";
  if (type || init)
    cout << " #";
  if (type)
    cout << " (type: " << type << ")";
  if (init)
    cout << " (default: \"" << init << "\")";
  cout << endl;
  return (true);
}
