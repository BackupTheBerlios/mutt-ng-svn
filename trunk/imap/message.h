/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-9 Brandon Long <blong@fiction.net>
 * Copyright (C) 1999-2000 Brendan Cully <brendan@kublai.com>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

/* message.c data structures */

#ifndef MESSAGE_H
#define MESSAGE_H 1

/* -- data structures -- */
/* IMAP-specific header data, stored as HEADER->data */
typedef struct imap_header_data {
  unsigned int uid;             /* 32-bit Message UID */
  LIST *keywords;
} IMAP_HEADER_DATA;

typedef struct {
  unsigned int read:1;
  unsigned int old:1;
  unsigned int deleted:1;
  unsigned int flagged:1;
  unsigned int replied:1;
  unsigned int changed:1;

  unsigned int sid;

  IMAP_HEADER_DATA *data;

  time_t received;
  long content_length;
} IMAP_HEADER;

/* -- macros -- */
#define HEADER_DATA(ph) ((IMAP_HEADER_DATA*) ((ph)->data))

#endif /* MESSAGE_H */
