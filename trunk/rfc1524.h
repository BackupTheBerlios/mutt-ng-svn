/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#ifndef _RFC1524_H
#define _RFC1524_H

typedef struct rfc1524_mailcap_entry {
  /*  char *contenttype; *//* we don't need this, as we search for it */
  char *command;
  char *testcommand;
  char *composecommand;
  char *composetypecommand;
  char *editcommand;
  char *printcommand;
  char *nametemplate;
  char *convert;
  /*  char *description; *//* we don't need this */
  unsigned int needsterminal:1; /* endwin() and system */
  unsigned int copiousoutput:1; /* needs pager, basically */
} rfc1524_entry;

rfc1524_entry *rfc1524_new_entry (void);
void rfc1524_free_entry (rfc1524_entry **);
int rfc1524_expand_command (BODY *, char *, char *, char *, int);
int rfc1524_expand_filename (char *, char *, char *, size_t);
int rfc1524_mailcap_lookup (BODY *, char *, rfc1524_entry *, int);
int mutt_rename_file (char *, char *);
int _mutt_rename_file (char *, char *, int);

#endif /* _RFC1524_H */
