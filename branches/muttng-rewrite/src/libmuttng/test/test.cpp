/**
 * @ingroup libmuttng
 * @addtogroup libmuttng_unit Unit Tests
 * @{
 */
/**
 * @file libmuttng/test/test.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: libmuttng/ unit tests
 */
#include <unit++/unit++.h>

using namespace unitpp;

#include "signal_tests.h"

namespace {

  /**
   * libmuttng/ unit testing class
   */
  class core_tests : public suite {
    public:
      core_tests() : suite("libmuttng_tests") {
        suite::main().add("libmuttng_test_suite",new signal_tests());
      }
  };

  /** main() */
  core_tests * theTest = new core_tests();

}

/** @} */
