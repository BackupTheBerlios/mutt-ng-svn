/**
 * @ingroup muttng_ui
 * @addtogroup muttng_ui_curses Curses
 * @{
 */
/**
 * @file muttng/ui/ui_curses.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: Curses-based UI
 *
 * This file is published under the GNU General Public License.
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
    UICurses (void);
    ~UICurses (void);
    bool start (void);
    bool end (void);
    bool enterFilename (void);
    bool enterPassword (void);
    bool answerQuestion (void);
    bool displayWarning (const buffer_t* message);
    bool displayError (const buffer_t* message);
    bool displayMessage (const buffer_t* message);
    bool displayProgress (const buffer_t* message);
    bool enterValue(buffer_t* dst, buffer_t* prompt, size_t dstlen);
    bool enterPassword(buffer_t* dst, buffer_t* prompt, size_t dstlen);
  private:
    /** whether curses is already running */
    bool isCurses;
};

#endif /* !MUTTNG_UI_CURSES_H */

/** @} */
