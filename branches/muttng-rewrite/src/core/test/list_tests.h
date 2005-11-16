#ifndef CORE_TEST_LIST__H
#define CORE_TEST_LIST__H

#include <unit++/unit++.h>
#include "list.h"

using namespace unitpp;

class list_tests : public suite {
    void test_pop1();
    void test_pop2();
    void test_pop3();
  public:
    list_tests();
};

#endif
