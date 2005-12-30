/* Define getaddrinfo to an innocuous variant, in case <limits.h> declares
 * getaddrinfo.
 *    For example, HP-UX 11i <limits.h> declares gettimeofday.  */
#define getaddrinfo innocuous_getaddrinfo

/* System header to define __stub macros and hopefully few prototypes,
 *     which can conflict with char getaddrinfo (); below.
 *         Prefer <limits.h> to <assert.h> if __STDC__ is defined, since
 *             <limits.h> exists even on freestanding compilers.  */

#ifdef __STDC__
# include <limits.h>
#else
# include <assert.h>
#endif

#undef getaddrinfo

/* Override any gcc2 internal prototype to avoid an error.  */
#ifdef __cplusplus
extern "C"
{
#endif

/* We use char because int might match the return type of a gcc2
 * builtin and then its argument prototype would still apply.  */
char getaddrinfo ();
/* The GNU C library defines this for functions which it implements
 * to always fail with ENOSYS.  Some functions are actually named
 * something starting with __ and the normal name is an alias.
 * */
#if defined (__stub_getaddrinfo) || defined (__stub___getaddrinfo)
choke me
#else
  char (*f) () = getaddrinfo;
#endif
#ifdef __cplusplus
}
#endif

int
main ()
{
  return f != getaddrinfo;
  ;
  return 0;
}
