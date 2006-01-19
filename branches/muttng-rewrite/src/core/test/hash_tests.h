/** @ingroup core_unit */
/**
 * @file core/test/hash_tests.h
 * @author Andreas Krennmair <ak@synflood.at>
 * @brief Interface: hash_t unit tests
 *
 * This file is published under the GNU General Public License.
 */
#ifndef CORE_TEST_HASH__H
#define CORE_TEST_HASH__H

#include <unit++/unit++.h>

using namespace unitpp;

/**
 * hash_t unit testing class
 */
class hash_tests : public suite {
    void test_bar();
  public:
    hash_tests();
};

#endif
