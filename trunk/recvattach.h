/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */
#ifndef _MUTT_RECVATTACH_H
#define _MUTT_RECVATTACH_H

#include "mutt_menu.h"

#include "attach.h"

void mutt_update_tree (ATTACHPTR **, short);

ATTACHPTR **mutt_gen_attach_list (BODY *, int, ATTACHPTR **, short *, short *,
                                  int, int);

const char *mutt_attach_fmt (char *dest,
                             size_t destlen,
                             char op,
                             const char *src,
                             const char *prefix,
                             const char *ifstring,
                             const char *elsestring,
                             unsigned long data, format_flag flags);

int mutt_tag_attach (MUTTMENU * menu, int n, int m);

int mutt_is_message_type (int, const char *);

void mutt_save_attachment_list (FILE * fp, int tag, BODY * top, HEADER * hdr,
                                MUTTMENU * menu);

void mutt_pipe_attachment_list (FILE * fp, int tag, BODY * top, int filter);
void mutt_print_attachment_list (FILE * fp, int tag, BODY * top);

int mutt_attach_display_loop (MUTTMENU * menu, int op, FILE * fp,
                              HEADER * hdr, BODY * cur, ATTACHPTR *** idxp,
                              short *idxlen, short *idxmax, int recv);

void mutt_attach_init (BODY *);

void mutt_view_attachments (HEADER *);

#endif /* !_MUTT_RECVATTACH_H */
