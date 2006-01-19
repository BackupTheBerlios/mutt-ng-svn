/** @ingroup libmuttng_unit */
/**
 * @file libmuttng/test/url_tests.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: URL unit tests
 *
 * This file is published under the GNU General Public License.
 */
#ifndef LIBMUTTNG_TEST_URL_TESTS_H
#define LIBMUTTNG_TEST_URL_TESTS_H

#include <unit++/unit++.h>

using namespace unitpp;

/**
 * url unit test
 */
class url_tests : public suite {
  public:
    url_tests();
  private:
    void test_invalid_proto();
    void test_parse();
    void test_decode();
};

#endif /* LIBMUTTNG_TEST_URL_TESTS_H */
