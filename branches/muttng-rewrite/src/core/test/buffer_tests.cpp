/** @ingroup core_unit */
/**
 * @file core/test/buffer_tests.cpp
 * @author Andreas Krennmair <ak@synflood.at>
 * @brief Implementation: buffer_t unit tests
 */
#include <unit++/unit++.h>
#include "buffer_tests.h"

using namespace unitpp;

void buffer_tests::test_foo() {
  int a = 1;
  assert_eq("assert description",1 /* expected expression result */,a /* expression */);
}

buffer_tests::buffer_tests() : suite("buffer_tests") {
  add("foo",testcase(this,"test_foo",&buffer_tests::test_foo));
}
