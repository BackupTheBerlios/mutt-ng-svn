/** @ingroup muttng_ui */
/**
 * @file muttng/ui/abstract_screen.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Abstract UI Screen superclass interface
 */
#ifndef MUTTNG_UI_ABSTRACT_SCREEN_H
#define MUTTNG_UI_ABSTRACT_SCREEN_H

#include "muttng.h"

/**
 * Screen abstraction superclass.
 */
class AbstractScreen : public Muttng {
  public:
    /** constructor */
    AbstractScreen (void);
    /** destructor */
    virtual ~AbstractScreen (void) = 0;
    /** states for AbstractScreen::getOp(). */
    enum state {
      /** key is unknown */
      T_UNKNOWN = 0,
      /** key is known, operation succeeded */
      T_KNOWN_OK,
      /** key is known, operation failed */
      T_KNOWN_ERROR
    };
    /**
     * Get key and return operation.
     * @return OP.
     */
    virtual AbstractScreen::state getOp (void) = 0;
    /** Init screen. */
    virtual void init (void) = 0;
};

#endif /* !MUTTNG_UI_ABSTRACT_SCREEN_H */
