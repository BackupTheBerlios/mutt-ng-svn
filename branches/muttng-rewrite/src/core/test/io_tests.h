/** @ingroup core_unit */
/**
 * @file core/test/io_tests.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: I/O unit tests
 *
 * This file is published under the GNU General Public License.
 */
#ifndef CORE_TEST_IO__H
#define CORE_TEST_IO__H

#include <unit++/unit++.h>
#include "io.h"
#include "buffer.h"

using namespace unitpp;

/**
 * io_*() unit testing class
 */
class io_tests : public suite {
    /**
     * Run a single test for io_tempfile().
     * @param dir @c dir Parameter for @c io_tempfile().
     * @param name @c name Parameter for @c io_tempfile().
     * @param tempfile @c tempfile Parameter for @c io_tempfile().
     * @return @c 1 on success, @c 0 on failure.
     */
    int test_io_tempfile2 (const char* dir, const char* name,
                           buffer_t* tempfile);
    /** run single test for @c io_open(). */
    void test_io_open (void);
    /** run several tests for @c io_tempfile(). */
    void test_io_tempfile (void);
    /** @test io_readling(). */
    void test_io_readline();
  public:
    io_tests(void);
};

#endif
