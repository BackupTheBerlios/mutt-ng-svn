#include <unit++/unit++.h>
#include "hash_tests.h"

using namespace unitpp;

void hash_tests::test_bar() {
	int a = 1;
	assert_eq("assert description",1 /* expected expression result */,a /* expression */);
}

hash_tests::hash_tests() : suite("hash_tests") {
	add("bar",testcase(this,"test_bar",&hash_tests::test_bar));
}
