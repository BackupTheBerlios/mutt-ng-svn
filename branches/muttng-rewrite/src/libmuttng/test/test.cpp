/**
 * @ingroup libmuttng
 * @addtogroup libmuttng_unit Unit Tests
 * @{
 */
/**
 * @file libmuttng/test/test.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: libmuttng/ unit tests
 *
 * This file is published under the GNU General Public License.
 */
#include <unit++/unit++.h>

using namespace unitpp;

#include "signal_tests.h"
#include "url_tests.h"
#include "conn_tests.h"
#include "header_tests.h"
#include "rfc2047_tests.h"

namespace {

  /**
   * libmuttng/ unit testing class
   */
  class libmuttng_tests : public suite {
    public:
      libmuttng_tests() : suite("libmuttng_tests") {
        suite::main().add("libmuttng_test_suite",new signal_tests());
        suite::main().add("libmuttng_test_suite",new url_tests());
        suite::main().add("libmuttng_test_suite",new conn_tests());
        suite::main().add("libmuttng_test_suite",new header_tests());
        suite::main().add("libmuttng_test_suite",new rfc2047_tests());
      }
  };

  /** main() */
  libmuttng_tests * theTest = new libmuttng_tests();

}

/** @} */
