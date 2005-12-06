/** @ingroup libmuttng_unit */
/**
 * @file libmuttng/test/header_tests.h
 * @author Andreas Krennmair <ak@synflood.at>
 * @brief Interface: Header unit tests
 */
#ifndef LIBMUTTNG_TEST_HEADER_TESTS_H
#define LIBMUTTNG_TEST_HEADER_TESTS_H

#include <unit++/unit++.h>

using namespace unitpp;

/**
 * Header unit test
 */
class header_tests : public suite {
  public:
    header_tests();
    ~header_tests();
  private:
    void test_constructors();
    void test_equalsname();
    void test_serialization();
    void test_parse();
};

#endif
