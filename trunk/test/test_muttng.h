#ifndef TEST_MUTTNG__H

#define mu_assert(message, test) do { asserts_run++; if (!(test)) return message; } while (0)
#define mu_run_test(test) do { char *message = test(); \
                               tests_run++; \
                               if (message) { \
                                 return message; \
                               } \
                             } while (0)
extern int tests_run;
extern int asserts_run;

#endif
