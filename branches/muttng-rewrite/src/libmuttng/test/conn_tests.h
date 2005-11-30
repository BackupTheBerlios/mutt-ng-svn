/** @ingroup libmuttng_unit */
/**
 * @file libmuttng/test/conn_tests.h
 * @author Andreas Krennmair <ak@synflood.at>
 * @brief Interface: Connection unit tests
 */
#ifndef LIBMUTTNG_TEST_CONN_TESTS_H
#define LIBMUTTNG_TEST_CONN_TESTS_H

#include <unit++/unit++.h>

using namespace unitpp;

/**
 * Connection unit test
 */
class conn_tests : public suite {
  public:
    conn_tests();
    ~conn_tests();
  private:
    void test_connectdisconnect();
    void test_readwrite();
};

#endif /* LIBMUTTNG_TEST_CONN_TESTS_H */
