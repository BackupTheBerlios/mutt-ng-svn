#include <unit++/unit++.h>
#include "list_tests.h"

using namespace unitpp;

void list_tests::test_pop1() {
  list_t* test = NULL;
  int pop = 42;
  list_push_back (&test, 23);
  pop = (int) list_pop_front (&test);
  assert_eq("list_pop_front() == 23", 23, pop);
  assert_eq("list = NULL", 1, test == NULL);
  pop = (int) list_pop_front (&test);
  assert_eq("list_pop_front() == 0", 0, pop);
}

void list_tests::test_pop2() {
  list_t* test = NULL;
  int pop = 42;
  list_push_back (&test, 23);
  pop = (int) list_pop_back (&test);
  assert_eq("list_pop_back() == 23", 23, pop);
  assert_eq("list = NULL", 1, test == NULL);
  pop = (int) list_pop_back (&test);
  assert_eq("list_pop_back() == 0", 0, pop);
}

void list_tests::test_pop3() {
  list_t* test = NULL;
  int pop = 42;
  list_push_back (&test, 23);
  pop = (int) list_pop_idx (&test, 0);
  assert_eq("list_pop_idx() == 23", 23, pop);
  assert_eq("list = NULL", 1, test == NULL);
  pop = (int) list_pop_idx (&test, 0);
  assert_eq("list_pop_idx() == 0", 0, pop);
}

list_tests::list_tests() : suite("list_tests") {
	add("pop1()",testcase(this,"test_pop1()",&list_tests::test_pop1));
	add("pop2()",testcase(this,"test_pop2()",&list_tests::test_pop2));
	add("pop3()",testcase(this,"test_pop3()",&list_tests::test_pop3));
}
