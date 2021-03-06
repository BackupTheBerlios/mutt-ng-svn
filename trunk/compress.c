/*
 * Copyright notice from original mutt:
 * Copyright (C) 1997 Alain Penders <Alain@Finale-Dev.com>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#include "mutt.h"

#ifdef USE_COMPRESSED

#include "mx.h"
#include "mbox.h"
#include "mutt_curses.h"

#include "lib/mem.h"
#include "lib/intl.h"
#include "lib/str.h"
#include "lib/debug.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

typedef struct {
  const char *close;            /* close-hook  command */
  const char *open;             /* open-hook   command */
  const char *append;           /* append-hook command */
  off_t size;                   /* size of real folder */
} COMPRESS_INFO;

char echo_cmd[HUGE_STRING];

/* parameters:
 * ctx - context to lock
 * excl - exclusive lock?
 * retry - should retry if unable to lock?
 */
int mbox_lock_compressed (CONTEXT * ctx, FILE * fp, int excl, int retry)
{
  int r;

  if ((r = mx_lock_file (ctx->realpath, fileno (fp), excl, 1, retry)) == 0)
    ctx->locked = 1;
  else if (retry && !excl) {
    ctx->readonly = 1;
    return 0;
  }

  return (r);
}

void mbox_unlock_compressed (CONTEXT * ctx, FILE * fp)
{
  if (ctx->locked) {
    fflush (fp);

    mx_unlock_file (ctx->realpath, fileno (fp), 1);
    ctx->locked = 0;
  }
}

static int is_new (const char *path)
{
  return (access (path, W_OK) != 0 && errno == ENOENT) ? 1 : 0;
}

static const char *find_compress_hook (int type, const char *path)
{
  const char *c = mutt_find_hook (type, path);

  return (!c || !*c) ? NULL : c;
}

int mutt_can_read_compressed (const char *path)
{
  return find_compress_hook (M_OPENHOOK, path) ? 1 : 0;
}

/* if the file is new, we really do not append, but create, and so use
 * close-hook, and not append-hook 
 */
static const char *get_append_command (const char *path, const CONTEXT * ctx)
{
  COMPRESS_INFO *ci = (COMPRESS_INFO *) ctx->compressinfo;

  return (is_new (path)) ? ci->close : ci->append;
}

int mutt_can_append_compressed (const char *path)
{
  int magic;

  if (is_new (path))
    return (find_compress_hook (M_CLOSEHOOK, path) ? 1 : 0);

  magic = mx_get_magic (path);

  if (magic != 0 && magic != M_COMPRESSED)
    return 0;

  return (find_compress_hook (M_APPENDHOOK, path)
          || (find_compress_hook (M_OPENHOOK, path)
              && find_compress_hook (M_CLOSEHOOK, path))) ? 1 : 0;
}

/* open a compressed mailbox */
static COMPRESS_INFO *set_compress_info (CONTEXT * ctx)
{
  COMPRESS_INFO *ci;

  /* Now lets uncompress this thing */
  ci = mem_malloc (sizeof (COMPRESS_INFO));
  ctx->compressinfo = (void *) ci;
  ci->append = find_compress_hook (M_APPENDHOOK, ctx->path);
  ci->open = find_compress_hook (M_OPENHOOK, ctx->path);
  ci->close = find_compress_hook (M_CLOSEHOOK, ctx->path);
  return ci;
}

static void set_path (CONTEXT * ctx)
{
  char tmppath[_POSIX_PATH_MAX];

  /* Setup the right paths */
  ctx->realpath = ctx->path;

  /* Uncompress to /tmp */
  mutt_mktemp (tmppath);
  ctx->path = mem_malloc (str_len (tmppath) + 1);
  strcpy (ctx->path, tmppath);
}

static int get_size (const char *path)
{
  struct stat sb;

  if (stat (path, &sb) != 0)
    return 0;
  return (sb.st_size);
}

static void store_size (CONTEXT * ctx)
{
  COMPRESS_INFO *ci = (COMPRESS_INFO *) ctx->compressinfo;

  ci->size = get_size (ctx->realpath);
}

static const char *compresshook_format_str (char *dest, size_t destlen,
                                            char op, const char *src,
                                            const char *fmt,
                                            const char *ifstring,
                                            const char *elsestring,
                                            unsigned long data,
                                            format_flag flags)
{
  char tmp[SHORT_STRING];

  CONTEXT *ctx = (CONTEXT *) data;

  switch (op) {
  case 'f':
    snprintf (tmp, sizeof (tmp), "%%%ss", fmt);
    snprintf (dest, destlen, tmp, ctx->realpath);
    break;
  case 't':
    snprintf (tmp, sizeof (tmp), "%%%ss", fmt);
    snprintf (dest, destlen, tmp, ctx->path);
    break;
  }
  return (src);
}

/* check that the command has both %f and %t
 * 0 means OK, -1 means error
 */
int mutt_test_compress_command (const char *cmd)
{
  return (strstr (cmd, "%f") && strstr (cmd, "%t")) ? 0 : -1;
}

static char *get_compression_cmd (const char *cmd, const CONTEXT * ctx)
{
  char expanded[_POSIX_PATH_MAX];

  mutt_FormatString (expanded, sizeof (expanded), cmd,
                     compresshook_format_str, (unsigned long) ctx, 0);
  return str_dup (expanded);
}

int mutt_check_mailbox_compressed (CONTEXT * ctx)
{
  COMPRESS_INFO *ci = (COMPRESS_INFO *) ctx->compressinfo;

  if (ci->size != get_size (ctx->realpath)) {
    mem_free (&ctx->compressinfo);
    mem_free (&ctx->realpath);
    mutt_error _("Mailbox was corrupted!");

    return (-1);
  }
  return (0);
}

int mutt_open_read_compressed (CONTEXT * ctx)
{
  char *cmd;
  FILE *fp;
  int rc;

  COMPRESS_INFO *ci = set_compress_info (ctx);

  if (!ci->open) {
    ctx->magic = 0;
    mem_free (ctx->compressinfo);
    return (-1);
  }
  if (!ci->close || access (ctx->path, W_OK) != 0)
    ctx->readonly = 1;

  set_path (ctx);
  store_size (ctx);

  if (!ctx->quiet)
    mutt_message (_("Decompressing %s..."), ctx->realpath);

  cmd = get_compression_cmd (ci->open, ctx);
  if (cmd == NULL)
    return (-1);
  debug_print (2, ("DecompressCmd: '%s'\n", cmd));

  if ((fp = fopen (ctx->realpath, "r")) == NULL) {
    mutt_perror (ctx->realpath);
    mem_free (&cmd);
    return (-1);
  }
  mutt_block_signals ();
  if (mbox_lock_compressed (ctx, fp, 0, 1) == -1) {
    fclose (fp);
    mutt_unblock_signals ();
    mutt_error _("Unable to lock mailbox!");

    mem_free (&cmd);
    return (-1);
  }

  endwin ();
  fflush (stdout);
  sprintf (echo_cmd, _("echo Decompressing %s..."), ctx->realpath);
  mutt_system (echo_cmd);
  rc = mutt_system (cmd);
  mbox_unlock_compressed (ctx, fp);
  mutt_unblock_signals ();
  fclose (fp);

  if (rc) {
    mutt_any_key_to_continue (NULL);
    ctx->magic = 0;
    mem_free (ctx->compressinfo);
    mutt_error (_("Error executing: %s : unable to open the mailbox!\n"),
                cmd);
  }
  mem_free (&cmd);
  if (rc)
    return (-1);

  if (mutt_check_mailbox_compressed (ctx))
    return (-1);

  ctx->magic = mx_get_magic (ctx->path);

  return (0);
}

void restore_path (CONTEXT * ctx)
{
  mem_free (&ctx->path);
  ctx->path = ctx->realpath;
}

/* remove the temporary mailbox */
void remove_file (CONTEXT * ctx)
{
  if (ctx->magic == M_MBOX || ctx->magic == M_MMDF)
    remove (ctx->path);
}

int mutt_open_append_compressed (CONTEXT * ctx)
{
  FILE *fh;
  COMPRESS_INFO *ci = set_compress_info (ctx);

  if (!get_append_command (ctx->path, ctx)) {
    if (ci->open && ci->close)
      return (mutt_open_read_compressed (ctx));

    ctx->magic = 0;
    mem_free (&ctx->compressinfo);
    return (-1);
  }

  set_path (ctx);

  ctx->magic = DefaultMagic;

  if (!is_new (ctx->realpath))
    if (ctx->magic == M_MBOX || ctx->magic == M_MMDF)
      if ((fh = safe_fopen (ctx->path, "w")))
        fclose (fh);
  /* No error checking - the parent function will catch it */

  return (0);
}

/* close a compressed mailbox */
void mutt_fast_close_compressed (CONTEXT * ctx)
{
  debug_print (2, ("called on '%s'\n", ctx->path));

  if (ctx->compressinfo) {
    if (ctx->fp)
      fclose (ctx->fp);
    ctx->fp = NULL;
    /* if the folder was removed, remove the gzipped folder too */
    if (access (ctx->path, F_OK) != 0 && !option (OPTSAVEEMPTY))
      remove (ctx->realpath);
    else
      remove_file (ctx);

    restore_path (ctx);
    mem_free (&ctx->compressinfo);
  }
}

/* return 0 on success, -1 on failure */
int mutt_sync_compressed (CONTEXT * ctx)
{
  char *cmd;
  int rc = 0;
  FILE *fp;
  COMPRESS_INFO *ci = (COMPRESS_INFO *) ctx->compressinfo;

  if (!ctx->quiet)
    mutt_message (_("Compressing %s..."), ctx->realpath);

  cmd = get_compression_cmd (ci->close, ctx);
  if (cmd == NULL)
    return (-1);

  if ((fp = fopen (ctx->realpath, "a")) == NULL) {
    mutt_perror (ctx->realpath);
    mem_free (&cmd);
    return (-1);
  }
  mutt_block_signals ();
  if (mbox_lock_compressed (ctx, fp, 1, 1) == -1) {
    fclose (fp);
    mutt_unblock_signals ();
    mutt_error _("Unable to lock mailbox!");

    store_size (ctx);

    mem_free (&cmd);
    return (-1);
  }

  debug_print (2, ("CompressCommand: '%s'\n", cmd));

  endwin ();
  fflush (stdout);
  sprintf (echo_cmd, _("echo Compressing %s..."), ctx->realpath);
  mutt_system (echo_cmd);
  if (mutt_system (cmd)) {
    mutt_any_key_to_continue (NULL);
    mutt_error (_
                ("%s: Error compressing mailbox! Original mailbox deleted, uncompressed one kept!\n"),
                ctx->path);
    rc = -1;
  }

  mbox_unlock_compressed (ctx, fp);
  mutt_unblock_signals ();
  fclose (fp);

  mem_free (&cmd);

  store_size (ctx);

  return (rc);
}

int mutt_slow_close_compressed (CONTEXT * ctx)
{
  FILE *fp;
  const char *append;
  char *cmd;
  COMPRESS_INFO *ci = (COMPRESS_INFO *) ctx->compressinfo;

  debug_print (2, ("called on '%s'\n", ctx->path));

  if (!(ctx->append && ((append = get_append_command (ctx->realpath, ctx))
                        || (append = ci->close)))) {    /* if we can not or should not append,
                                                         * we only have to remove the compressed info, because sync was already
                                                         * called 
                                                         */
    mutt_fast_close_compressed (ctx);
    return (0);
  }

  if (ctx->fp)
    fclose (ctx->fp);
  ctx->fp = NULL;

  if (!ctx->quiet) {
    if (append == ci->close)
      mutt_message (_("Compressing %s..."), ctx->realpath);
    else
      mutt_message (_("Compressed-appending to %s..."), ctx->realpath);
  }

  cmd = get_compression_cmd (append, ctx);
  if (cmd == NULL)
    return (-1);

  if ((fp = fopen (ctx->realpath, "a")) == NULL) {
    mutt_perror (ctx->realpath);
    mem_free (&cmd);
    return (-1);
  }
  mutt_block_signals ();
  if (mbox_lock_compressed (ctx, fp, 1, 1) == -1) {
    fclose (fp);
    mutt_unblock_signals ();
    mutt_error _("Unable to lock mailbox!");

    mem_free (&cmd);
    return (-1);
  }

  debug_print (2, ("CompressCmd: '%s'\n", cmd));

  endwin ();
  fflush (stdout);

  if (append == ci->close)
    sprintf (echo_cmd, _("echo Compressing %s..."), ctx->realpath);
  else
    sprintf (echo_cmd, _("echo Compressed-appending to %s..."),
             ctx->realpath);
  mutt_system (echo_cmd);

  if (mutt_system (cmd)) {
    mutt_any_key_to_continue (NULL);
    mutt_error (_
                (" %s: Error compressing mailbox!  Uncompressed one kept!\n"),
                ctx->path);
    mem_free (&cmd);
    mbox_unlock_compressed (ctx, fp);
    mutt_unblock_signals ();
    fclose (fp);
    return (-1);
  }

  mbox_unlock_compressed (ctx, fp);
  mutt_unblock_signals ();
  fclose (fp);
  remove_file (ctx);
  restore_path (ctx);
  mem_free (&cmd);
  mem_free (&ctx->compressinfo);

  return (0);
}

mx_t* compress_reg_mx (void) {
  mx_t* fmt = mem_calloc (1, sizeof (mx_t));
  fmt->type = M_COMPRESSED;
  fmt->local = 1;
  fmt->mx_is_magic = mbox_is_magic;
  fmt->mx_check_empty = mbox_check_empty;
  fmt->mx_access = access;
  fmt->mx_open_mailbox = mutt_open_read_compressed;
  return (fmt);
}

#endif /* USE_COMPRESSED */
