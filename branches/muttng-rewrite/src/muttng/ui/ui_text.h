/** @ingroup muttng */
/**
 * @file muttng/ui/ui_text.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Text-based UI superclass interface
 */
#ifndef MUTTNG_UI_TEXT_H
#define MUTTNG_UI_TEXT_H

#include "ui.h"

/**
 * Text-based user interface.
 * This is a specialization of the generic UI for text-based, i.e.
 * console output. It only implements the UI::displayVersion() and
 * UI::displayWarranty() functions as only these are common to all CLI
 * tools.
 */
class UIText : public UI {
  public:
    UIText ();
    virtual ~UIText ();
    virtual bool start (void) = 0;
    virtual bool end (void) = 0;
    virtual bool enterFilename (void) = 0;
    virtual bool enterPassword (void) = 0;
    virtual bool answerQuestion (void) = 0;
    virtual bool displayError (void) = 0;
    virtual void displayMessage (const char* message) = 0;
    void displayVersion (const char* name, const char* copyright,
                         const char* options, const char* reach);
    void displayWarranty (const char* name, const char* copyright,
                          const char* license, const char* reach);
};

#endif /* !MUTTNG_UI_TEXT_H */
