/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#define SORT_DATE	1       /* the date the mail was sent. */
#define SORT_SIZE	2
#define SORT_SUBJECT	3
#define SORT_ALPHA	3       /* makedoc.c requires this */
#define SORT_FROM	4
#define SORT_ORDER	5       /* the order the messages appear in the mailbox. */
#define SORT_THREADS	6
#define SORT_RECEIVED	7       /* when the message were delivered locally */
#define SORT_TO		8
#define SORT_SCORE	9
#define SORT_ALIAS	10
#define SORT_ADDRESS	11
#define SORT_KEYID	12
#define SORT_TRUST	13
#define SORT_SPAM	14
/* dgc: Sort & SortAux are shorts, so I'm bumping these bitflags up from
 * bits 4 & 5 to bits 8 & 9 to make room for more sort keys in the future. */
#define SORT_MASK	0xff
#define SORT_REVERSE	(1<<8)
#define SORT_LAST	(1<<9)

typedef int sort_t (const void *, const void *);
sort_t *mutt_get_sort_func (int);

void mutt_sort_headers (CONTEXT *, int);
int mutt_select_sort (int);

WHERE short BrowserSort INITVAL (SORT_SUBJECT);
WHERE short Sort INITVAL (SORT_DATE);
WHERE short SortAux INITVAL (SORT_DATE);        /* auxiallary sorting method */
WHERE short SortAlias INITVAL (SORT_ALIAS);

/* FIXME: This one does not belong to here */
WHERE short PgpSortKeys INITVAL (SORT_ADDRESS);

#include "mapping.h"
extern const struct mapping_t SortMethods[];
