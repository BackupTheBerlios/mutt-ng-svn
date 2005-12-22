/** @ingroup core_unit */
/**
 * @file core/test/net_tests.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: IDNA unit tests
 */
#ifndef CORE_TEST_NET__H
#define CORE_TEST_NET__H

#include <unit++/unit++.h>

using namespace unitpp;

/**
 * conversion unit testing class
 */
class net_tests : public suite {
  void test_to_local();
  void test_from_local();
  public:
    net_tests();
    ~net_tests();
};

#endif
