#include "net.h"
#include "str.h"
#include "io.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>


#ifndef STDC_HEADERS
int fclose ();
#endif

/* poor man's version of getdomainname() for systems where it does not return
 * return the DNS domain, but the NIS domain.
 */

static void strip_trailing_dot (char *q)
{
  char *p = q;

  for (; *q; q++)
    p = q;

  if (*p == '.')
    *p = '\0';
}

int net_dnsdomainname (buffer_t* dst) {
  FILE *f;
  char tmp[1024];
  char *p = NULL;
  char *q;

  if (!dst)
    return 0;

  if ((f = fopen ("/etc/resolv.conf", "r")) == NULL)
    return 0;

  tmp[sizeof (tmp) - 1] = 0;

  while (fgets (tmp, sizeof (tmp) - 1, f) != NULL) {
    p = tmp;
    while (ISSPACE (*p))
      p++;
    if (str_ncmp ("domain", p, 6) == 0
        || str_ncmp ("search", p, 6) == 0) {
      p += 6;

      for (q = strtok (p, " \t\n"); q; q = strtok (NULL, " \t\n"))
        if (strcmp (q, "."))
          break;

      if (q) {
        strip_trailing_dot (q);
        buffer_add_str(dst,q,-1);
        io_fclose (&f);
        return 1;
      }

    }
  }

  io_fclose (&f);
  return 0;
}
