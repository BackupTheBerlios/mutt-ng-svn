/**
 * @ingroup libmuttng
 * @addtogroup libmuttng_crypto Cryptography
 * @{
 */
/**
 * @file libmuttng/crypto/crypto.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: Message cryptography
 */
#include "libmuttng/libmuttng.h"

/**
 * Base class for crypto abstraction.
 */
class Crypto : public LibMuttng {
  public:
    Crypto (void);
    virtual ~Crypto (void) = 0;
    /** dummy */
    virtual void pgpFoo (void) = 0;
    /** dummy */
    virtual void smimeFoo (void) = 0;
};

/** @} */
