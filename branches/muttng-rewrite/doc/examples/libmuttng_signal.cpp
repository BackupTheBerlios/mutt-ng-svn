
/* Example: signal handling */
#include <iostream>
#include "libmuttng/muttng_signal.h"

class Sig {
  public:
    /* signal takes 1 argument of type 'int' */
    Signal1<int> testSignal;
};

class Handler {
  public:
    /* this will be connected to a signal with 1 argument of type 'int' */
    bool handle (int a) {
      std::cout << "a = " << a << std::endl;
      return true;
    }
};

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
