/** @ingroup muttng_ui_cli */
/**
 * @file muttng/ui/plain_screen_motion.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Plain UI Screen superclass interface
 */
#ifndef MUTTNG_UI_PLAIN_SCREEN_MOTION_H
#define MUTTNG_UI_PLAIN_SCREEN_MOTION_H

#include "abstract_screen_motion.h"

/**
 * Dummy class with NOOP motions for CLI.
 */
class PlainScreenMotion : public AbstractScreenMotion {
  public:
    PlainScreenMotion (void);
    virtual ~PlainScreenMotion (void) = 0;
    bool setFirst (void);
    bool setLast (void);
};

#endif /* !MUTTNG_UI_PLAIN_SCREEN_MOTION_H */
