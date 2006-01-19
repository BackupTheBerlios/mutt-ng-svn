/** @ingroup core_unit */
/**
 * @file core/test/buffer_tests.h
 * @author Andreas Krennmair <ak@synflood.at>
 * @brief Interface: buffer_t unit tests
 *
 * This file is published under the GNU General Public License.
 */
#ifndef CORE_TEST_BUFFER__H
#define CORE_TEST_BUFFER__H

#include <unit++/unit++.h>

using namespace unitpp;

/**
 * buffer_t unit testing class
 */
class buffer_tests : public suite {
    void test_buffer_init();
    void test_buffer_shrink();
    void test_buffer_grow();
    void test_buffer_equal1();
    void test_buffer_add_str();
    void test_buffer_add_buffer();
    void test_buffer_add_ch();
    void test_buffer_add_snum2();
    void test_buffer_add_unum2();
    void test_buffer_chomp();
    void test_buffer_tokenize();
    void test_buffer_format();
  public:
    buffer_tests();
};

#endif
