/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

/* common protos for compose / attach menus */

int mutt_tag_attach (MUTTMENU * menu, int n, int m);
int mutt_attach_display_loop (MUTTMENU * menu, int op, FILE * fp,
                              HEADER * hdr, BODY * cur, ATTACHPTR *** idxp,
                              short *idxlen, short *idxmax, int recv);


void mutt_save_attachment_list (FILE * fp, int tag, BODY * top, HEADER * hdr,
                                MUTTMENU * menu);
void mutt_pipe_attachment_list (FILE * fp, int tag, BODY * top, int filter);
void mutt_print_attachment_list (FILE * fp, int tag, BODY * top);

void mutt_attach_bounce (FILE *, HEADER *, ATTACHPTR **, short, BODY *);
void mutt_attach_resend (FILE *, HEADER *, ATTACHPTR **, short, BODY *);
void mutt_attach_forward (FILE *, HEADER *, ATTACHPTR **, short, BODY *, int);
void mutt_attach_reply (FILE *, HEADER *, ATTACHPTR **, short, BODY *, int);
