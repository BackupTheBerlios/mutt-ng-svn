/** @ingroup muttng */
/**
 * @file muttng/ui/ui_curses.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Curses-based UI interface
 */
#ifndef MUTTNG_UI_CURSES_H
#define MUTTNG_UI_CURSES_H

#include "ui_text.h"

/**
 * Curses- or SLang-based user interface.
 * For quite some time (maybe forever), this will be the only real
 * interactive user interface we have.
 */
class UICurses : public UIText {
  public:
    UICurses ();
    ~UICurses ();
    bool start (void);
    bool end (void);
    bool enterFilename (void);
    bool enterPassword (void);
    bool answerQuestion (void);
    bool displayError (void);
    void displayMessage (const char* message);
  private:
    /** whether curses is already running */
    bool isCurses;
};

#endif /* !MUTTNG_UI_CURSES_H */
