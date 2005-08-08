/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */
#ifndef _MUTT_ATTACH_H
#define _MUTT_ATTACH_H

#include "mutt_menu.h"

typedef struct attachptr {
  BODY *content;
  int parent_type;
  char *tree;
  int level;
  int num;
  unsigned int unowned:1;       /* don't unlink on detach */
} ATTACHPTR;

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

ATTACHPTR **mutt_gen_attach_list (BODY *, int, ATTACHPTR **, short *, short *,
                                  int, int);

void mutt_update_tree (ATTACHPTR **, short);

int mutt_print_attachment (FILE *, BODY *);
int mutt_decode_save_attachment (FILE *, BODY *, char *, int, int);
int mutt_save_attachment (FILE *, BODY *, char *, int, HEADER *);
int mutt_pipe_attachment (FILE *, BODY *, const char *, char *);
int mutt_view_attachment (FILE *, BODY *, int, HEADER *, ATTACHPTR **, short);
int mutt_is_autoview (BODY *, const char *);
void mutt_check_lookup_list (BODY *, char *, int);
int mutt_edit_attachment (BODY *);
int mutt_compose_attachment (BODY * a);
int mutt_get_tmp_attachment (BODY *);

#endif /* !_MUTT_ATTACH_H */
