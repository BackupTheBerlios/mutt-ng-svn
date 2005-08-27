/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */
#ifndef _MUTT_THREAD_H
#define _MUTT_THREAD_H

#define mutt_next_thread(x) _mutt_aside_thread(x,1,0)
#define mutt_previous_thread(x) _mutt_aside_thread(x,0,0)
#define mutt_next_subthread(x) _mutt_aside_thread(x,1,1)
#define mutt_previous_subthread(x) _mutt_aside_thread(x,0,1)
int _mutt_aside_thread (HEADER *, short, short);

#define mutt_collapse_thread(x,y) _mutt_traverse_thread (x,y,M_THREAD_COLLAPSE)
#define mutt_uncollapse_thread(x,y) _mutt_traverse_thread (x,y,M_THREAD_UNCOLLAPSE)
#define mutt_get_hidden(x,y)_mutt_traverse_thread (x,y,M_THREAD_GET_HIDDEN)
#define mutt_thread_contains_unread(x,y) _mutt_traverse_thread (x,y,M_THREAD_UNREAD)
#define mutt_thread_next_unread(x,y) _mutt_traverse_thread(x,y,M_THREAD_NEXT_UNREAD)
int _mutt_traverse_thread (CONTEXT * ctx, HEADER * hdr, int flag);

void mutt_clear_threads (CONTEXT *);

void mutt_sort_threads (CONTEXT *, int);
THREAD *mutt_sort_subthreads (THREAD *, int);

int mutt_parent_message (CONTEXT *, HEADER *);
void mutt_set_virtual (CONTEXT *);

int mutt_messages_in_thread (CONTEXT *, HEADER *, int);

HASH *mutt_make_id_hash (CONTEXT *);
HASH *mutt_make_subj_hash (CONTEXT *);

int mutt_link_threads (HEADER *, HEADER *, CONTEXT *);
void mutt_break_thread (HEADER *);

/* computes an envelope's real_subj from subj */
void mutt_adjust_subject (ENVELOPE* e);
/* does mutt_adjust_subject() for messages in Context */
void mutt_adjust_all_subjects (void);

#endif /* !_MUTT_THREAD_H */
