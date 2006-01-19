/** @ingroup core_unit */
/**
 * @file core/test/list_tests.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: list_t unit tests
 *
 * This file is published under the GNU General Public License.
 */
#ifndef CORE_TEST_LIST__H
#define CORE_TEST_LIST__H

#include <unit++/unit++.h>
#include "list.h"

using namespace unitpp;

/**
 * list_t unit testing class
 */
class list_tests : public suite {
    /** test for list_pop_front() */
    void test_pop_front();
    /** test for list_pop_back() */
    void test_pop_back();
    /** test for list_pop_idx() */
    void test_pop_idx();
  public:
    list_tests();
};

#endif
