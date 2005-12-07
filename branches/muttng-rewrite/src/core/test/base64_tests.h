/** @ingroup libmuttng_unit */
/**
 * @file libmuttng/test/base64_tests.h
 * @author Andreas Krennmair <ak@synflood.at>
 * @brief Interface: base64 unit tests
 */

#ifndef LIBMUTTNG_TEST_BASE64_TESTS_H
#define LIBMUTTNG_TEST_BASE64_TESTS_H

#include <unit++/unit++.h>

using namespace unitpp;


/**
 * base64 unit test
 */

class base64_tests : public suite {
  public:
    base64_tests();
    ~base64_tests();
  private:
    void test_encode();
    void test_decode();
    void test_both();
};

#endif
