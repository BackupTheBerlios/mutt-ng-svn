/** @ingroup libmuttng_unit */
/**
 * @file core/test/base64_tests.h
 * @author Andreas Krennmair <ak@synflood.at>
 * @brief Interface: base64 unit tests
 *
 * This file is published under the GNU General Public License.
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
    /** @test buffer_encode_base64(). */
    void test_encode();
    /** @test buffer_decode_base64(). */
    void test_decode();
    /**
     * @test buffer_decode_base64().
     * @test buffer_encode_base64().
     */
    void test_both();
};

#endif
