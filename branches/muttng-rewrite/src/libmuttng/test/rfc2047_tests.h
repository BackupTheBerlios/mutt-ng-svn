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
#include "libmuttng/util/rfc2047.h"

using namespace unitpp;

/**
 * RfC2047 tests.
 */
class rfc2047_tests : public suite, private RfC2047 {
  public:
    rfc2047_tests();
    ~rfc2047_tests();
  private:
    /** @test rfc2047_decode(). */
    void test_decode();
    /** @test rfc2047_decode(). */
    void test_decode_buf(buffer_t* dst);
    size_t isclean(buffer_t* src);
};

#endif /* LIBMUTTNG_TEST_RFC2047_TESTS_H */
