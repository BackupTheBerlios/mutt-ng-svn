#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>

int main(void) {
  int sshort = sizeof(unsigned short),
      sint = sizeof(unsigned int),
      slong = sizeof(unsigned long);
  printf("#if HAVE_C99_INTTYPES\n"
         "typedef unsigned %s uint32_t;\n"
         "typedef unsigned %s uint64_t;\n"
         "#endif\n",
         (sshort==4 ? "short" : (sint==4 ? "int" : "long" )),
         (sint==8 ? "int" : (slong==8 ? "long" : "long long")));
  return 0;
}
