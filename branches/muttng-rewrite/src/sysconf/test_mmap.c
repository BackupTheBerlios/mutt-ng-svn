#include <sys/types.h>
#include <sys/mman.h>
int main (void) { void* p = mmap (0,0,0,0,0,0); (void) p; return (0); }
