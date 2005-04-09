/*
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mutt.h"
#include "imap_private.h"

#include "mx.h"
#include "mx_imap.h"

#include "lib/mem.h"
#include "lib/str.h"

#include "url.h"

static int imap_is_magic (const char* path) {
  url_scheme_t s;
  if (!path || !*path)
    return (-1);
  if (*path == '{')     /* painpine compatibility */
    return (M_IMAP);
  s = url_check_scheme (NONULL (path));
  return ((s == U_IMAP || s == U_IMAPS) ? M_IMAP : -1);
}

static int acl_check_imap (CONTEXT* ctx, int bit) {
  return (!mutt_bit_isset (((IMAP_DATA*) ctx->data)->capabilities, ACL) ||
          mutt_bit_isset (((IMAP_DATA*) ctx->data)->rights, bit));
}

mx_t* imap_reg_mx (void) {
  mx_t* fmt = safe_calloc (1, sizeof (mx_t));

  /* make up mx_t record... */
  fmt->type = M_IMAP;
  fmt->mx_is_magic = imap_is_magic;
  fmt->mx_access = imap_access;
  fmt->mx_open_mailbox = imap_open_mailbox;
  fmt->mx_acl_check = acl_check_imap;
  return (fmt);
}
