

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mutt.h"
#include "nntp.h"

#include "mx.h"
#include "mx_nntp.h"

#include "lib/mem.h"
#include "lib/str.h"

#include "url.h"

static int nntp_is_magic (const char* path) {
  url_scheme_t s = url_check_scheme (NONULL (path));
  return ((s == U_NNTP || s == U_NNTPS) ? M_NNTP : -1);
}

/* called by nntp_init(); don't call elsewhere */
mx_t* nntp_reg_mx (void) {
  mx_t* fmt = safe_calloc (1, sizeof (mx_t));

  /* make up mx_t record... */
  fmt->type = M_NNTP;
  fmt->mx_is_magic = nntp_is_magic;
  fmt->mx_open_mailbox = nntp_open_mailbox;
  return (fmt);
}
