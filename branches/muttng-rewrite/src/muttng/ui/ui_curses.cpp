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

bool UICurses::displayError (const buffer_t* message) {
  if (isCurses) {
    /* former mutt error */
  } else if (message)
    cerr << message->str << endl;
  return true;
}

bool UICurses::displayMessage (const buffer_t* message) {
  if (isCurses) {
    /* former mutt_message() */
  } else if (message)
    cout << message->str << endl;
  /* sleep(1) */
  return true;
}

bool UICurses::displayProgress (const buffer_t* message) {
  if (isCurses) {
    /* former mutt_message() */
  } else if (message)
    cout << message->str << endl;
  return true;
}

bool UICurses::enterValue(buffer_t* dst, buffer_t* prompt, size_t dstlen) {
  (void)dst;(void)prompt;(void)dstlen;
  return false;
}

bool UICurses::enterPassword(buffer_t* dst, buffer_t* prompt, size_t dstlen) {
  (void)dst;(void)prompt;(void)dstlen;
  return false;
}
