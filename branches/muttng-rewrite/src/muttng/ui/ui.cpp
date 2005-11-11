/** @ingroup muttng_ui */
/**
 * @file muttng/ui/ui.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief UI superclass implementation
 */
#include "ui.h"

UI::UI (ConfigScreen* configScreen) {
  this->configScreen = configScreen;
}
UI::~UI (void) {
  delete (this->configScreen);
}

ConfigScreen* UI::getConfigScreen (void) {
  return (this->configScreen);
}
