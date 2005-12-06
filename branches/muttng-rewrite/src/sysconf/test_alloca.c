#ifdef FREEBSD
#include <stdlib.h>
#else
#include "alloca.h"
#endif
int main (void) { char* p = alloca (10); (void) p; return (0); }
