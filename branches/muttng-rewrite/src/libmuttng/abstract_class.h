/** @ingroup libmuttng */
/**
 * @file libmuttng/abstract_class.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Abstract base class.
 */
#ifndef LIBMUTTNG_ABSTRACT_CLASS_H
#define LIBMUTTNG_ABSTRACT_CLASS_H

/**
 * Abstract base class for all objects providing debug functionality.
 */
class AbstractClass {
  public:
    AbstractClass (void);
    ~AbstractClass (void);
    static void debugSetLevel (int level);
    static bool debugStart (void);
    static bool debugEnd (void);
    static bool debugPrint (const char* file, int line,
                            const char* func, const char* msg);
    static bool debugPrint (const char* file, int line,
                            const char* func, int num);
  private:
    static int fd;
};

#ifdef __GNUG__
#define DEBUGPRINT(X)   this->debugPrint(__FILE__,__LINE__,__FUNCTION__,X)
#else
#define DEBUGPRINT(X)   this->debugPrint(__FILE__,__LINE__,NULL,X)
#endif

#endif /* !LIBMUTTNG_ABSTRACT_CLASS_H */

/** @} */
