/** @ingroup muttng_ui_curses */
/**
 * @file muttng/ui/ui_curses.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: Curses-based UI
 */
#include <iostream>

#include "ui_curses.h"
#include "ui_plain.h"

using namespace std;

UICurses::UICurses (void) {
  this->isCurses = false;
}

UICurses::~UICurses (void) {}

bool UICurses::start (void) { return (true); }
bool UICurses::end (void) { return (true); }
bool UICurses::enterFilename (void) { return (true); }
bool UICurses::enterPassword (void) { return (true); }
bool UICurses::answerQuestion (void) { return (true); }

void UICurses::displayError (const char* message) {
  if (isCurses) {
    /* former mutt error */
  } else if (message)
    cerr << message << endl;
}

void UICurses::displayMessage (const char* message) {
  if (isCurses) {
    /* former mutt_message() */
  } else if (message)
    cout << message << endl;
}
