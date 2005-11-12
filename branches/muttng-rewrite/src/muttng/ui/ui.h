/* vim:foldmethod=marker:foldlevel=0:
 */
/* doxygen documentation {{{ */
/**
 * @ingroup muttng
 * @addtogroup muttng_ui User Interfaces
 * @{
 *
 * @section muttng_ui_intro Introduction
 *
 *   All mechanisms are push-based.
 *
 *   When an event is emitted
 *   somewhere within the application (even if's only displaying usage),
 *   all necessary data is pushed into the UI rather than passing the
 *   event to the UI and letting it pull the data as required.
 *
 * @section muttn_ui_menus How menus work
 *
 *   All user interfaces work with a push-based mechanism, too. That means
 *   that for each type of screen we know, there's a class derived from
 *   AbstractScreen.
 *
 *   For every user interface, this derived class must be
 *   implemented (or at least as far as the tools based on it need it)
 *   by the UI and passed up through hierarchy constructors up to the
 *   abstract UI class (e.g. from UIPlain via UIText up to UI).
 *
 *   When entering a screen, the core logic pushes all data it has into
 *   the class registered into UI via the constructors. Classes have the
 *   chance to some arbitrary initialization first.
 *
 *   Over a pull-based mechanism (being standard as I was told), this
 *   has the advantage that from the core's point of view, the UI is
 *   fully replaceable and that we don't need to implement similar
 *   pull-based logic for all UIs. The drawback is that the implementing
 *   UI has to buffer all data independ on what it actually needs. Also,
 *   this adds some complexity but adds structure even with the "core"
 *   UI separating it into "display-related" and "core logic."
 *
 *   As things like motion within screens is common for all UIs or can
 *   be made abstract, there's the AbstractScreenMotion class.
 *
 *   A particular UI needs to implement:
 *     -# one the children of AbstractScreen (e.g. ConfigScreen)
 *     -# a derivate of AbstractScreenMotion
 *
 *   As ConfigScreen and the like declare the motion commands pure
 *   virtual, the UI must inherit from both, from it's implemented
 *   AbstractScreen-based class and from it's implemented motion class
 *   to actually complete the signature for AbstractScreen.
 *
 *   For UIs like UICurses, the motion can be devided into two groups:
 *   one line-based (e.g. index) and one page-based (e.g. help.) Thus,
 *   the screen display logic goes into the AbstractScreen derivate
 *   while the motion logic is down the path from AbstractScreenMotion.
 *   For example, if we ever decided to make the help screen line-based
 *   we only need to change the inheritance for the curses help screen.
 *   If we need to make the index page-based, we change the inheritance
 *   from CursesScreenLineMotion to CursesScreenPageMotion and that
 *   would be it.
 *
 * @section muttng_ui_concl Conclusion
 *
 *   This concept is quite difficult to understand but it's
 *   the best of way of really separating the UI from the core logic we
 *   came up with so far. Thus, this maybe is subject to change
 *   completely.
 */
/* }}} */
/**
 * @file muttng/ui/ui.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief UI superclass interface.
 */
#ifndef MUTTNG_UI_H
#define MUTTNG_UI_H

#include <stdlib.h>

#include "muttng.h"
#include "config_screen.h"

/**
 * User interface superclass.
 * The Tool class and its children only rely on these abstract
 * functions.
 */
class UI : public Muttng {
  public:
    /** constructor */
    UI (ConfigScreen* configScreen = NULL);
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
    /**
     * Get pointer to UI's config screen.
     * @return Pointer.
     */
    ConfigScreen* getConfigScreen (void);
  private:
    /** UI's config screen */
    ConfigScreen* configScreen;
};

#endif /* !MUTTNG_UI_H */

/** @} */
