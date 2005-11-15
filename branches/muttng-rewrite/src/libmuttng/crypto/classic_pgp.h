/** @inggroup libmuttng_crypto */
/**
 * @file libmuttng/crypto/classic_pgp.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: Classic PGP for ClassicCrypto
 */
#ifndef LIBMUTTNG_CRYPTO_CLASSIC_PGP_H
#define LIBMUTTNG_CRYPTO_CLASSIC_PGP_H

#include "../libmuttng.h"

/**
 * Base class for crypto abstraction.
 */
class ClassicPGP : public LibMuttng {
  public:
    ClassicPGP (void);
    ~ClassicPGP (void);

    void pgpFoo (void);

};

#endif /* !LIBMUTTNG_CRYPTO_CLASSIC_PGP_H */
