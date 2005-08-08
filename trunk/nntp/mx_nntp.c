/*
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/stat.h>

#include "mutt.h"
#include "nntp.h"

#include "mx.h"
#include "mx_nntp.h"

#include "lib/mem.h"
#include "lib/str.h"

#include "url.h"

static int nntp_is_magic (const char* path, struct stat* st) {
  url_scheme_t s = url_check_scheme (NONULL (path));
  return ((s == U_NNTP || s == U_NNTPS) ? M_NNTP : -1);
}

static int acl_check_nntp (CONTEXT* ctx, int bit) {
  switch (bit) {
    case ACL_INSERT:    /* editing messages */
    case ACL_WRITE:     /* change importance */
      return (0);
    case ACL_DELETE:    /* (un)deletion */
    case ACL_SEEN:      /* mark as read */
      return (1);
    default:
      return (0);
  }
}

/* called by nntp_init(); don't call elsewhere */
mx_t* nntp_reg_mx (void) {
  mx_t* fmt = mem_calloc (1, sizeof (mx_t));

  /* make up mx_t record... */
  fmt->type = M_NNTP;
  fmt->mx_is_magic = nntp_is_magic;
  fmt->mx_open_mailbox = nntp_open_mailbox;
  fmt->mx_acl_check = acl_check_nntp;
  fmt->mx_fastclose_mailbox = nntp_fastclose_mailbox;
  fmt->mx_sync_mailbox = nntp_sync_mailbox;
  fmt->mx_check_mailbox = nntp_check_mailbox;
  return (fmt);
}
