/*
 * Copyright notice from original mutt:
 * Copyright (C) 1997 Alain Penders <Alain@Finale-Dev.com>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

int mutt_can_read_compressed (const char *);
int mutt_can_append_compressed (const char *);
int mutt_open_read_compressed (CONTEXT *);
int mutt_open_append_compressed (CONTEXT *);
int mutt_slow_close_compressed (CONTEXT *);
int mutt_sync_compressed (CONTEXT *);
int mutt_test_compress_command (const char *);
int mutt_check_mailbox_compressed (CONTEXT *);
void mutt_fast_close_compressed (CONTEXT *);
