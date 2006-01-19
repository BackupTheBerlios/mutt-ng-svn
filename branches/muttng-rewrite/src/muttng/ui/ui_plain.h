/**
 * @ingroup muttng_ui
 * @addtogroup muttng_ui_cli Command-line
 * @{
 * */
/**
 * @file muttng/ui/ui_plain.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: CLI user interface
 *
 * This file is published under the GNU General Public License.
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
    /**
     * Create new "plain" UI.
     * @param name_ Name of application for error reporting.
     */
    UIPlain (const char* name_);
    ~UIPlain (void);
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
    /** application name */
    const char* name;
};

#endif /* !MUTTNG_UI_PLAIN_H */

/** @} */
