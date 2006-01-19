/** @ingroup libmuttng_crypto */
/**
 * @file libmuttng/crypto/crypto_gpgme.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: GPGME support for Crypto
 *
 * This file is published under the GNU General Public License.
 */
#ifndef LIBMUTTNG_CRYPTO_CRYPTO_GPGME_H
#define LIBMUTTNG_CRYPTO_CRYPTO_GPGME_H

#include "libmuttng/crypto/crypto.h"

/**
 * Base class for crypto abstraction.
 */
class CryptoGPGMe : public Crypto {
  public:
    CryptoGPGMe (void);
    ~CryptoGPGMe (void);

    void pgpFoo (void);

    void smimeFoo (void);
};

#endif /* !LIBMUTTNG_CRYPTO_CRYPTO_GPGME_H */
