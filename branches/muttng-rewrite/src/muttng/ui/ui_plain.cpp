/** @ingroup muttng_ui_cli */
/**
 * @file muttng/ui/ui_plain.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Plain UI implementation.
 */
#include <iostream>

#include "ui_plain.h"

using namespace std;

UIPlain::UIPlain (void) : UIText (new PlainConfigScreen ()) {
}

UIPlain::~UIPlain (void) {
}

bool UIPlain::start (void) { return (true); }
bool UIPlain::end (void) { return (true); }
bool UIPlain::enterFilename (void) { return (true); }
bool UIPlain::enterPassword (void) { return (true); }
bool UIPlain::answerQuestion (void) { return (true); }
bool UIPlain::displayError (void) { return (true); }
void UIPlain::displayMessage (const char* message) {
  if (message)
    cout << message << endl;
}
