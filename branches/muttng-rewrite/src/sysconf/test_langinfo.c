#include <langinfo.h>

int main() { char* p = nl_langinfo(CODESET); (void)p; return 0; }
