/* based on MinUnit */
#include <stdio.h>
#include "test_muttng.h"

int tests_run = 0;

static char * test_case(void) {
  mu_assert("NULL != 0",NULL == 0);
  return 0;
}

static char * all_tests(void) {
  mu_run_test(test_case);
  return 0;
}

int main(int argc, char **argv) {
  char *result = all_tests();
  if (result != 0) {
    printf("ERROR: %s\n", result);
  } else {
    printf("ALL TESTS PASSED\n");
  }
  printf("Tests run: %d\n", tests_run);
  return result != 0;
}
