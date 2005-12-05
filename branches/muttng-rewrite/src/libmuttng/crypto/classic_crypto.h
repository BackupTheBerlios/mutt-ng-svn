/** @ingroup libmuttng_crypto */
/**
 * @file libmuttng/crypto/classic_crypto.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: Classic module for Crypto
 */
#ifndef LIBMUTTNG_CRYPTO_CLASSIC_CRYPTO_H
#define LIBMUTTNG_CRYPTO_CLASSIC_CRYPTO_H

#include "libmuttng/crypto/crypto.h"
#include "libmuttng/crypto/classic_pgp.h"
#include "libmuttng/crypto/classic_smime.h"

/**
 * Base class for crypto abstraction.
 */
class ClassicCrypto : public ClassicPGP, public ClassicSMIME, public Crypto {
  public:
    ClassicCrypto (void);
    ~ClassicCrypto (void);
};

#endif /* !LIBMUTTNG_CRYPTO_CLASSIC_CRYPTO_H */
