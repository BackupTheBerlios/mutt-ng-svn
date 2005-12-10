/** @ingroup libmuttng_unit */
/**
 * @file libmuttng/test/rfc2047_tests.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: Signal unit tests
 */
#ifndef LIBMUTTNG_TEST_RFC2047_TESTS_H
#define LIBMUTTNG_TEST_RFC2047_TESTS_H

#include <unit++/unit++.h>

#include "core/buffer.h"

using namespace unitpp;

/**
 * RfC2047 tests.
 */
class rfc2047_tests : public suite {
  public:
    rfc2047_tests();
  private:
    /** @test rfc2047_decode(). */
    void test_decode();
    /** @test rfc2047_decode(). */
    void test_decode_buf(buffer_t* dst);
    size_t isclean(buffer_t* src);
};

#endif /* LIBMUTTNG_TEST_RFC2047_TESTS_H */
