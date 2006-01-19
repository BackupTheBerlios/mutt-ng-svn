
/**
 * @file examples/libmuttng_signal.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Example: How to use libmuttng's signals
 *
 * This file is published as public domain.
 */
#include <iostream>
#include "libmuttng/muttng_signal.h"

/**
 * Dummy class containing only a signal declaration.
 */
class Sig {
  public:
    /** testSignal takes 1 argument of type 'int' */
    Signal1<int> testSignal;
};

/**
 * Handler class with a method elsewhere connected to a signal.
 */
class Handler {
  public:
    /** 
     * Signal handler.
     * This will be connected to a signal with 1 argument of type 'int'.
     * @param a The argument passed to Signal1::emit().
     * @return true
     */
    bool handle (int a) {
      std::cout << "a = " << a << std::endl;
      return true;
    }
};

/**
 * @c main().
 * @return 1.
 */
int main (void) {
  Sig sig;
  Handler* handler = new Handler;

  /* connect handler::handle() to sig::testSignal */
  connectSignal (sig.testSignal, handler, &Handler::handle);

  /*
   * emit test signal with different arguments.
   * as handler::handle() is called, it will print:
   *    a = 1
   *    a = 2
   *    a = 3
   */
  sig.testSignal.emit (1);
  sig.testSignal.emit (2);
  sig.testSignal.emit (3);

  /*
   * for test signals, remove all signal handlers registered
   * for 'handler' object
   * in this case, this only is handler::handle()
   */
  disconnectSignals (sig.testSignal, handler);

  delete handler;
  return 0;
}
