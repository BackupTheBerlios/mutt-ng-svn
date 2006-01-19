/** @ingroup core_unit */
/**
 * @file core/test/list_tests.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: list_t unit tests
 *
 * This file is published under the GNU General Public License.
 */
#include <unit++/unit++.h>
#include "list_tests.h"

using namespace unitpp;

void list_tests::test_pop_front () {
  list_t* test = NULL;
  int pop = 42;
  list_push_back (&test, 23);
  pop = (int) list_pop_front (&test);
  assert_eq("list_pop_front() == 23", 23, pop);
  assert_eq("list = NULL", 1, test == NULL);
  pop = (int) list_pop_front (&test);
  assert_eq("list_pop_front() == 0", 0, pop);
}

void list_tests::test_pop_back () {
  list_t* test = NULL;
  int pop = 42;
  list_push_back (&test, 23);
  pop = (int) list_pop_back (&test);
  assert_eq("list_pop_back() == 23", 23, pop);
  assert_eq("list = NULL", 1, test == NULL);
  pop = (int) list_pop_back (&test);
  assert_eq("list_pop_back() == 0", 0, pop);
}

void list_tests::test_pop_idx () {
  list_t* test = NULL;
  int pop = 42;
  list_push_back (&test, 23);
  pop = (int) list_pop_idx (&test, 0);
  assert_eq("list_pop_idx() == 23", 23, pop);
  assert_eq("list = NULL", 1, test == NULL);
  pop = (int) list_pop_idx (&test, 0);
  assert_eq("list_pop_idx() == 0", 0, pop);
}

list_tests::list_tests() : suite("list_tests") {
  add("list_pop_front()",testcase(this,"test_pop_fron()",&list_tests::test_pop_front));
  add("list_pop_back()",testcase(this,"test_pop_back()",&list_tests::test_pop_back));
  add("list_pop_idx()",testcase(this,"test_pop_idx()",&list_tests::test_pop_idx));
}
