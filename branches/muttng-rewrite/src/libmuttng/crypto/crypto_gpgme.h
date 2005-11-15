/** @ingroup libmuttng_crypto */
/**
 * @file libmuttng/crypto/crypto_gpgme.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: GPGME support for Crypto
 */
#ifndef LIBMUTTNG_CRYPTO_CRYPTO_GPGME_H
#define LIBMUTTNG_CRYPTO_CRYPTO_GPGME_H

#include "crypto.h"

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
