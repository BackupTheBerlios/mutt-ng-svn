/** @inggroup libmuttng_crypto */
/**
 * @file libmuttng/crypto/classic_smime.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: Classic S/MIME for ClassicCrypto
 *
 * This file is published under the GNU General Public License.
 */
#ifndef LIBMUTTNG_CRYPTO_CLASSIC_SMIME_H
#define LIBMUTTNG_CRYPTO_CLASSIC_SMIME_H

#include "libmuttng/libmuttng.h"

/**
 * Base class for crypto abstraction.
 */
class ClassicSMIME : public LibMuttng {
  public:
    ClassicSMIME (void);
    ~ClassicSMIME (void);
    /** @copydoc Crypto::smimeFoo(). */
    void smimeFoo (void);

};

#endif /* !LIBMUTTNG_CRYPTO_CLASSIC_SMIME_H */
