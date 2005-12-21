/** @ingroup libmuttng_unit */
/**
 * @file libmuttng/test/lib_tests.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: Libmuttng base unit testing class
 */
#ifndef LIBMUTTNG_TEST_LIB_TESTS_H
#define LIBMUTTNG_TEST_LIB_TESTS_H

#include <unit++/unit++.h>

#include "core/buffer.h"

#include "libmuttng/libmuttng.h"

using namespace unitpp;

/**
 * Base class for unit testing. This just catches libmuttng's message
 * signals to print to @c stderr.
 */
class lib_tests : public LibMuttng {
  public:
    lib_tests();
    ~lib_tests();
  private:
    bool displayText (const buffer_t* message);
};

#endif /* LIBMUTTNG_TEST_CONN_TESTS_H */
