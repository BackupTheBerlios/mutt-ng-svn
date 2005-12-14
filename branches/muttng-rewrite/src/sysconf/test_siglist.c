#include <signal.h>
/* NetBSD declares sys_siglist in unistd.h.  */
#if HAVE_UNISTD_H
# include <unistd.h>
#endif

int main () {
#ifndef sys_siglist
  char *p = (char *) sys_siglist; (void)p;
#endif
  return 0;
}
