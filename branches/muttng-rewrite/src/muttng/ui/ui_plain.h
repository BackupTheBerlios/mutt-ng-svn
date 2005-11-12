/**
 * @ingroup muttng_ui
 * @addtogroup muttng_ui_cli Command-line
 * @{
 * */
/**
 * @file muttng/ui/ui_plain.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Plain UI interface
 */
#ifndef MUTTNG_UI_PLAIN_H
#define MUTTNG_UI_PLAIN_H

#include "ui_text.h"

/**
 * Plain user interface.
 * This is used by all non-interactive tools which simply print text to
 * the console. Thus, some functions are simply dummies, i.e. most of
 * which prompt for input.
 */
class UIPlain : public UIText {
  public:
    UIPlain (void);
    ~UIPlain (void);
    bool start (void);
    bool end (void);
    bool enterFilename (void);
    bool enterPassword (void);
    bool answerQuestion (void);
    void displayError (const char* message);
    void displayMessage (const char* message);
};

#endif /* !MUTTNG_UI_PLAIN_H */

/** @} */
