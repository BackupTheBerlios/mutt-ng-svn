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
#include "imap_private.h"

#include "mx.h"
#include "mx_imap.h"

#include "lib/mem.h"
#include "lib/str.h"

#include "url.h"

int imap_is_magic (const char* path, struct stat* st) {
  url_scheme_t s;
  if (!path || !*path)
    return (-1);
  s = url_check_scheme (NONULL (path));
  return ((s == U_IMAP || s == U_IMAPS) ? M_IMAP : -1);
}

static int acl_check_imap (CONTEXT* ctx, int bit) {
  return (!mutt_bit_isset (((IMAP_DATA*) ctx->data)->capabilities, ACL) ||
          mutt_bit_isset (((IMAP_DATA*) ctx->data)->rights, bit));
}

static int imap_open_new_message (MESSAGE * msg, CONTEXT * dest, HEADER * hdr)
{
  char tmp[_POSIX_PATH_MAX];

  mutt_mktemp (tmp);
  if ((msg->fp = safe_fopen (tmp, "w")) == NULL) {
    mutt_perror (tmp);
    return (-1);
  }
  msg->path = str_dup (tmp);
  return 0;
}

/* this ugly kludge is required since the last int to
 * imap_check_mailbox() doesn't mean 'lock' but 'force'... */
static int _imap_check_mailbox (CONTEXT* ctx, int* index_hint, int lock) {
  return (imap_check_mailbox (ctx, index_hint, 0));
}

static int imap_commit_message (MESSAGE* msg, CONTEXT* ctx) {
  int r = 0;

  if ((r = safe_fclose (&msg->fp)) == 0)
    r = imap_append_message (ctx, msg);
  return (r);
}

mx_t* imap_reg_mx (void) {
  mx_t* fmt = safe_calloc (1, sizeof (mx_t));

  /* make up mx_t record... */
  fmt->type = M_IMAP;
  fmt->mx_is_magic = imap_is_magic;
  fmt->mx_access = imap_access;
  fmt->mx_open_mailbox = imap_open_mailbox;
  fmt->mx_open_new_message = imap_open_new_message;
  fmt->mx_acl_check = acl_check_imap;
  fmt->mx_fastclose_mailbox = imap_close_mailbox;
  fmt->mx_sync_mailbox = imap_sync_mailbox;
  fmt->mx_check_mailbox = _imap_check_mailbox;
  fmt->mx_commit_message = imap_commit_message;
  return (fmt);
}
