/** @ingroup muttng_ui */
/**
 * @file muttng/ui/abstract_screen_motion.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Abstract UI Screen superclass interface
 */
#ifndef MUTTNG_UI_ABSTRACT_SCREEN_MOTION_H
#define MUTTNG_UI_ABSTRACT_SCREEN_MOTION_H

/**
 * Screen motion abstraction */
class AbstractScreenMotion {
  public:
    /** constructor */
    AbstractScreenMotion (void);
    /** destructor */
    virtual ~AbstractScreenMotion (void) = 0;
    /**
     * Set pointer to first item.
     * @return Success.
     */
    virtual bool setFirst (void) = 0;
    /**
     * Set pointer to last item.
     * @return Success.
     */
    virtual bool setLast (void) = 0;
};

#endif /* !MUTTNG_UI_ABSTRACT_SCREEN_MOTION_H */
