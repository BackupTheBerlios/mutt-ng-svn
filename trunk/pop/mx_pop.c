#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mutt.h"
#include "pop.h"

#include "mx.h"
#include "mx_pop.h"

#include "lib/mem.h"
#include "lib/str.h"

#include "url.h"

static int pop_is_magic (const char* path) {
  url_scheme_t s = url_check_scheme (NONULL (path));
  return ((s == U_POP || s == U_POPS) ? M_POP : -1);
}

mx_t* pop_reg_mx (void) {
  mx_t* fmt = safe_calloc (1, sizeof (mx_t));

  /* make up mx_t record... */
  fmt->type = M_POP;
  fmt->mx_is_magic = pop_is_magic;
  fmt->mx_open_mailbox = pop_open_mailbox;
  return (fmt);
}
