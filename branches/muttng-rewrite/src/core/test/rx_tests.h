/** @ingroup core_unit */
/**
 * @file core/test/rx_tests.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: Regular expression unit tests
 *
 * This file is published under the GNU General Public License.
 */
#ifndef CORE_TEST_RX__H
#define CORE_TEST_RX__H

#include <unit++/unit++.h>

using namespace unitpp;

/**
 * regex unit testing class
 */
class rx_tests : public suite {
  private:
    /** @test rx_compile(). */
    void test_rx_compile();
    /** @test rx_match(). */
    void test_rx_match();
  public:
    rx_tests();
    ~rx_tests();
};

#endif
