/**
 * @ingroup core
 * @addtogroup core_unit Unit Tests
 * @{
 */
/**
 * @file core/test/test.cpp
 * @author Andreas Krennmair <ak@synflood.at>
 * @brief Implementation: core/ unit tests
 */
#include <unit++/unit++.h>

using namespace unitpp;

#include "buffer_tests.h"
#include "hash_tests.h"
#include "list_tests.h"
#include "io_tests.h"
#include "conv_tests.h"

namespace {

  /**
   * core/ unit testing class
   */
  class core_tests : public suite {
    public:
      core_tests() : suite("core_tests") {
        suite::main().add("core_test_suite",new buffer_tests());
        suite::main().add("core_test_suite",new hash_tests());
        suite::main().add("core_test_suite",new list_tests());
        suite::main().add("core_test_suite",new io_tests());
        suite::main().add("core_test_suite",new conv_tests());
      }
  };

  /** main() */
  core_tests * theTest = new core_tests();

}

/** @} */
