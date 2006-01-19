/** @ingroup libmuttng_unit */
/**
 * @file libmuttng/test/signal_tests.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: Signal unit tests
 *
 * This file is published as public domain.
 */
#include <unit++/unit++.h>
#include "signal_tests.h"

using namespace unitpp;

void signal_tests::test_signal_connect (void) {
  int a = 0, b = 0;

  /** - connect signal1 handlers to signal_tests::sig1 */
  connectSignal (sig1, test1, &testSig1::sig1_handler_1);
  connectSignal (sig1, test2, &testSig2::sig1_handler_2);
  connectSignal (sig1, test1, &testSig1::sig1_handler_3);
  connectSignal (sig1, test2, &testSig2::sig1_handler_1);
  connectSignal (sig1, test1, &testSig1::sig1_handler_2);
  connectSignal (sig1, test2, &testSig2::sig1_handler_3);
  /** - all handlers must succeed */
  assert_true ("sig1: all handlers succeeded", sig1.emit (&a));
  /** - we connected 6 handlers and all must have been called */
  assert_eq ("sig1: 6 handlers called", 6, a);

  a = 0;
  /** - connect signal2 handlers to signal_tests::sig2 */
  connectSignal (sig2, test1, &testSig1::sig2_handler_1);
  connectSignal (sig2, test2, &testSig2::sig2_handler_2);
  connectSignal (sig2, test1, &testSig1::sig2_handler_3);
  connectSignal (sig2, test2, &testSig2::sig2_handler_1);
  connectSignal (sig2, test1, &testSig1::sig2_handler_2);
  connectSignal (sig2, test2, &testSig2::sig2_handler_3);
  /** - all handlers must succeed */
  assert_true ("sig2: all handlers succeeded", sig2.emit (&a, b));
  /** - we connected 6 handlers and all must have been called */
  assert_eq ("sig2: 6 handlers called", 6, a);
}

void signal_tests::test_signal_loop (void) {
  int a = 0, b = 0;

  /**
   * - first, connect signal_test::sig1_loop() and
   *   signal_tests::sig2_loop() which themselves emit
   *   signal_tests::sig1 signal_test2::sig2 to trigger
   *   loops
   */
  connectSignal (sig1, this, &signal_tests::sig1_loop);
  connectSignal (sig2, this, &signal_tests::sig2_loop);

  /** - second, emitting sig1 must fail due to loop */
  assert_true ("sig1: loop detected", !sig1.emit (&a));
  /** - third, emitting sig2 must fail due to loop */
  assert_true ("sig2: loop detected", !sig2.emit (&a, b));
  /** - fourth, disconnect loops from signals */
  disconnectSignals (sig1, this);
  disconnectSignals (sig2, this);
}

void signal_tests::test_signal_disconnect (void) {
  int a = 0, b = 0;

  /** - sig1: disconnect 3 signal_tests::testSig1 handlers */
  disconnectSignals (sig1, test1);
  /** - sig1: all remaining from signal_tests::testSig2 must succeed */
  assert_true ("sig1: all handlers succeeded", sig1.emit (&a));
  /** - sig1: there must be three from signal_tests::testSig2 */
  assert_eq ("sig1: 3 handlers called", 3, a);

  a = 0;
  /** - sig1: disconnect all remaining so that none are connected */
  disconnectSignals (sig1, test2);
  /** - sig1: emitting it must succeed though no handlers */
  assert_true ("sig1: all handlers succeeded", sig1.emit (&a));
  /** - sig1: the emit must not have called any handler */
  assert_eq ("sig1: 0 handlers called", 0, a);

  a = 0;
  /** - sig2: disconnect 3 signal_tests::testSig1 handlers */
  disconnectSignals (sig2, test1);
  /** - sig2: all remaining from signal_tests::testSig2 must succeed */
  assert_true ("sig2: all handlers succeeded", sig2.emit (&a, b));
  /** - sig2: there must be three from signal_tests::testSig2 */
  assert_eq ("sig2: 3 handlers called", 3, a);

  a = 0;
  /** - sig2: disconnect all remaining so that none are connected */
  disconnectSignals (sig2, test2);
  /** - sig2: emitting it must succeed though no handlers */
  assert_true ("sig2: all handlers succeeded", sig2.emit (&a, b));
  /** - sig2: the emit must not have called any handler */
  assert_eq ("sig2: 0 handlers called", 0, a);
}

signal_tests::signal_tests() : suite("signal_tests") {
  test1 = new testSig1;
  test2 = new testSig2;
  add("signal",testcase(this,"test_signal_connect",
                        &signal_tests::test_signal_connect));
  add("signal",testcase(this,"test_signal_loop",
                        &signal_tests::test_signal_loop));
  add("signal",testcase(this,"test_signal_disconnect",
                        &signal_tests::test_signal_disconnect));
}

signal_tests::~signal_tests() {
  delete test1;
  delete test2;
}

bool signal_tests::sig1_loop(int * a) {
      if ((*a) > 10)
        return (false);
      return (sig1.emit(&(++(*a))));
}

bool signal_tests::sig2_loop (int* a, int b) {
  if ((*a) > 10)
    return (false);
  return (sig2.emit(&(++(*a)), b));
}
