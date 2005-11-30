/** @ingroup libmuttng_unit */
/**
 * @file libmuttng/test/signal_tests.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: Signal unit tests
 */
#ifndef LIBMUTTNG_TEST_SIGNAL_TESTS_H
#define LIBMUTTNG_TEST_SIGNAL_TESTS_H

#include <unit++/unit++.h>
#include "../muttng_signal.h"

using namespace unitpp;

/** test class with handlers for sig1 and sig2 */
class testSig1 { /* {{{ */
  public:
    bool sig1_handler_1 (int* a) { (*a)++; return (true); }
    bool sig1_handler_2 (int* a) { (*a)++; return (true); }
    bool sig1_handler_3 (int* a) { (*a)++; return (true); }
    bool sig2_handler_1 (int* a, int b) { (void) b; (*a)++; return (true); }
    bool sig2_handler_2 (int* a, int b) { (void) b; (*a)++; return (true); }
    bool sig2_handler_3 (int* a, int b) { (void) b; (*a)++; return (true); }
}; /* }}} */

/** test class with handlers for sig1 and sig2 */
class testSig2 { /* {{{ */
  public:
    bool sig1_handler_1 (int* a) { (*a)++; return (true); }
    bool sig1_handler_2 (int* a) { (*a)++; return (true); }
    bool sig1_handler_3 (int* a) { (*a)++; return (true); }
    bool sig2_handler_1 (int* a, int b) { (void) b; (*a)++; return (true); }
    bool sig2_handler_2 (int* a, int b) { (void) b; (*a)++; return (true); }
    bool sig2_handler_3 (int* a, int b) { (void) b; (*a)++; return (true); }
}; /* }}} */

/**
 * signal unit test
 */
class signal_tests : public suite {
  public:
    signal_tests();
    ~signal_tests();
  private:
    /** test signal with 1 parameter */
    Signal1<int*> sig1;
    /** test signal with 2 parameters */
    Signal2<int*,int> sig2;
    /** instance for testing */
    testSig1* test1;
    /** instance for testing */
    testSig2* test2;

    /** trigger loop in sig1 emission */
    bool sig1_loop (int* a);
    /** trigger loop in sig2 emission */
    bool sig2_loop (int* a, int b);

    /** test for connectSignal() */
    void test_signal_connect();
    /** test for emit() */
    void test_signal_loop();
    /** test for disconnectSignals() */
    void test_signal_disconnect();
};

#endif /* LIBMUTTNG_TEST_SIGNAL_TESTS_H */
