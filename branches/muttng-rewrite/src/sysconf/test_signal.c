#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#ifdef signal
# undef signal
#endif
#ifdef __cplusplus
extern "C" void (*signal (int, void (*)(int)))(int);
#else
void (*signal ()) ();
#endif

int main () {
  puts("/** return type of signal handlers */\n#define CORE_SIGTYPE      void");
  return 0;
}
