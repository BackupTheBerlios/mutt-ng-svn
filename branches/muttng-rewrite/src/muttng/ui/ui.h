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
     * Prompt user to answer some question.
     * @b NOTE: This still is a dummy without proper parameters but
     *    leaves the option to add a GUI with specialized dialogs.
     * @return Success.
     */
    virtual bool answerQuestion (void) = 0;
    /**
     * Display an error message to user.
     * @param message Error message.
     */
    virtual void displayError (const char* message) = 0;
    /**
     * Display a normal message to user.
     * @param message The message's text.
     */
    virtual void displayMessage (const char* message) = 0;
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
