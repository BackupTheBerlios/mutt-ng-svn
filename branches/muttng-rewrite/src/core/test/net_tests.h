/** @ingroup core_unit */
/**
 * @file core/test/net_tests.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: IDNA unit tests
 *
 * This file is published under the GNU General Public License.
 */
#ifndef CORE_TEST_NET__H
#define CORE_TEST_NET__H

#include <unit++/unit++.h>

using namespace unitpp;

/**
 * conversion unit testing class
 */
class net_tests : public suite {
  /** @test net_idn2local(). */
  void test_to_local();
  /** @test net_local2idn(). */
  void test_from_local();
  public:
    net_tests();
    ~net_tests();
};

#endif
