/* vim:foldmethod=marker:foldlevel=0:
 */
/**
 * @ingroup muttng
 * @addtogroup muttng_ui User Interfaces
 * @{
 */
/**
 * @file muttng/ui/ui.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: UI base class
 *
 * This file is published under the GNU General Public License.
 */
#ifndef MUTTNG_UI_H
#define MUTTNG_UI_H

#include <stdlib.h>

#include "muttng.h"

/**
 * User interface superclass.
 * The Tool class and its children only rely on these abstract
 * functions.
 */
class UI : public Muttng {
  public:
    /** constructor */
    UI (void);
    /** destructor */
    virtual ~UI (void);
    /**
     * Initialize UI.
     * @return Success.
     */
    virtual bool start (void) = 0;
    /**
     * Cleanup UI after run.
     * @return Success.
     */
    virtual bool end (void) = 0;
    /**
     * Prompt user for filename.
     * @b NOTE: This still is a dummy without proper parameters but
     *    leaves the option to add a GUI with specialized dialogs.
     * @return Success.
     */
    virtual bool enterFilename (void) = 0;
    /**
     * Prompt user for password.
     * @b NOTE: This still is a dummy without proper parameters but
     *    leaves the option to add a GUI with specialized dialogs.
     * @return Success.
     */
    virtual bool enterPassword (void) = 0;
    /**
     * Prompt user to answer some yes/no question.
     * @b NOTE: This still is a dummy without proper parameters but
     *    leaves the option to add a GUI with specialized dialogs.
     * @return Success.
     */
    virtual bool answerQuestion (void) = 0;
    /**
     * Prompt the user to enter a value.
     * @param dst Destination buffer for answer.
     * @param prompt Text prompt.
     * @return Success.
     * @bug remove dstlen
     */
    virtual bool enterValue(buffer_t* dst, buffer_t* prompt, size_t dstlen) = 0;
    /**
     * Prompt the user to enter a password.
     * @param dst Destination buffer for answer.
     * @param prompt Text prompt.
     * @return Success.
     * @bug remove dstlen
     */
    virtual bool enterPassword(buffer_t* dst, buffer_t* prompt, size_t dstlen) = 0;
    /**
     * Display an error message to user.
     * @param message Error message.
     * @return true
     */
    virtual bool displayError (const buffer_t* message) = 0;

    /**
     * Display a warning message to user.
     * @param message Error message.
     * @return true
     */
    virtual bool displayWarning (const buffer_t* message) = 0;

    /**
     * Display a normal message to user to see.
     * @param message The message's text.
     * @return true
     */
    virtual bool displayMessage (const buffer_t* message) = 0;
    /**
     * Display a normal message to user not necessarily to see.
     * @param message The message's text.
     * @return true
     */
    virtual bool displayProgress (const buffer_t* message) = 0;
    /**
     * Display version information to user.
     * @param name Name and version info of the Tool.
     * @param copyright The copyright string.
     * @param options List of compile-time options.
     * @param reach How to contact us.
     */
    virtual void displayVersion (const char* name,
                                 const char* copyright,
                                 const char* options,
                                 const char* reach) = 0;
    /**
     * Display warranty and legal stuff to user.
     * @param name Name and version info of the Tool.
     * @param copyright The copyright string.
     * @param license Short license text.
     * @param reach How to contact us.
     */
    virtual void displayWarranty (const char* name,
                                  const char* copyright,
                                  const char* license,
                                  const char* reach) = 0;
 };

#endif /* !MUTTNG_UI_H */

/** @} */
