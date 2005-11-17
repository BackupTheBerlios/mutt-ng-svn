/** @ingroup core_unit */
/**
 * @file core/test/list_tests.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: list_t unit tests
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
    void test_pop1();
    void test_pop2();
    void test_pop3();
  public:
    list_tests();
};

#endif
