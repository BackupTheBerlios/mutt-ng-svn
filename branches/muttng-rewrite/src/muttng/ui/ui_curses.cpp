/** @ingroup muttng_ui_curses */
/**
 * @file muttng/ui/ui_curses.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Curses-based UI implementation
 */
#include <iostream>

#include "ui_curses.h"

using namespace std;

UICurses::UICurses (ConfigScreen* configScreen) : UIText (configScreen) {
  this->isCurses = false;
}

UICurses::~UICurses (void) {}

bool UICurses::start (void) { return (true); }
bool UICurses::end (void) { return (true); }
bool UICurses::enterFilename (void) { return (true); }
bool UICurses::enterPassword (void) { return (true); }
bool UICurses::answerQuestion (void) { return (true); }
bool UICurses::displayError (void) { return (true); }

void UICurses::displayMessage (const char* message) {
  if (isCurses) {
    /* former mutt_message() */
  } else if (message)
    cout << message << endl;
}
