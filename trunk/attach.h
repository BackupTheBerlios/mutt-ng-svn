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

typedef struct attachptr {
  BODY *content;
  int parent_type;
  char *tree;
  int level;
  int num;
  unsigned int unowned:1;       /* don't unlink on detach */
} ATTACHPTR;

void mutt_attach_bounce (FILE *, HEADER *, ATTACHPTR **, short, BODY *);
void mutt_attach_resend (FILE *, HEADER *, ATTACHPTR **, short, BODY *);
void mutt_attach_forward (FILE *, HEADER *, ATTACHPTR **, short, BODY *, int);
void mutt_attach_reply (FILE *, HEADER *, ATTACHPTR **, short, BODY *, int);

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

/* this checks whether at least one text line matches
 * AttachRemindRegexp and returns 1 if so and 0 otherwise */
int mutt_attach_check (HEADER* hdr);

#endif /* !_MUTT_ATTACH_H */
