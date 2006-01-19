/** @ingroup libmuttng_unit */
/**
 * @file libmuttng/test/conn_tests.h
 * @author Andreas Krennmair <ak@synflood.at>
 * @brief Interface: Connection unit tests
 *
 * This file is published under the GNU General Public License.
 */
#ifndef LIBMUTTNG_TEST_CONN_TESTS_H
#define LIBMUTTNG_TEST_CONN_TESTS_H

#include <unit++/unit++.h>

#include "core/buffer.h"

#include "lib_tests.h"
#include "libmuttng/util/url.h"

using namespace unitpp;

/**
 * Connection unit test
 */
class conn_tests : public suite, public lib_tests {
  public:
    conn_tests();
    ~conn_tests();
  private:
    /** url for testing */
    url_t* url;
    void init();
    void test_connectdisconnect();
    void test_readwrite();
    void test_canread();
};

#endif /* LIBMUTTNG_TEST_CONN_TESTS_H */
