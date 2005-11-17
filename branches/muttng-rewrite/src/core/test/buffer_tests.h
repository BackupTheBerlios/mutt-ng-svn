/** @ingroup core_unit */
/**
 * @file core/test/buffer_tests.h
 * @author Andreas Krennmair <ak@synflood.at>
 * @brief Interface: buffer_t unit tests
 */
#ifndef CORE_TEST_BUFFER__H
#define CORE_TEST_BUFFER__H

#include <unit++/unit++.h>

using namespace unitpp;

/**
 * buffer_t unit testing class
 */
class buffer_tests : public suite {
    void test_foo();
  public:
    buffer_tests();
};

#endif
