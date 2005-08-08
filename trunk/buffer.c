/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */
#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include "buffer.h"

#include "lib/mem.h"
#include "lib/str.h"

/*
 * Creates and initializes a BUFFER*. If passed an existing BUFFER*,
 * just initializes. Frees anything already in the buffer.
 *
 * Disregards the 'destroy' flag, which seems reserved for caller.
 * This is bad, but there's no apparent protocol for it.
 */
BUFFER *mutt_buffer_init (BUFFER * b)
{
  if (!b) {
    b = mem_malloc (sizeof (BUFFER));
    if (!b)
      return NULL;
  }
  else {
    mem_free(&b->data);
  }
  memset (b, 0, sizeof (BUFFER));
  return b;
}

/*
 * Creates and initializes a BUFFER*. If passed an existing BUFFER*,
 * just initializes. Frees anything already in the buffer. Copies in
 * the seed string.
 *
 * Disregards the 'destroy' flag, which seems reserved for caller.
 * This is bad, but there's no apparent protocol for it.
 */
BUFFER *mutt_buffer_from (BUFFER * b, char *seed)
{
  if (!seed)
    return NULL;

  b = mutt_buffer_init (b);
  b->data = str_dup (seed);
  b->dsize = str_len (seed);
  b->dptr = (char *) b->data + b->dsize;
  return b;
}

void mutt_buffer_addstr (BUFFER * buf, const char *s)
{
  mutt_buffer_add (buf, s, str_len (s));
}

void mutt_buffer_addch (BUFFER * buf, char c)
{
  mutt_buffer_add (buf, &c, 1);
}

void mutt_buffer_free (BUFFER ** p)
{
  if (!p || !*p)
    return;

  mem_free (&(*p)->data);
  /* dptr is just an offset to data and shouldn't be freed */
  mem_free (p);
}

/* dynamically grows a BUFFER to accomodate s, in increments of 128 bytes.
 * Always one byte bigger than necessary for the null terminator, and
 * the buffer is always null-terminated */
void mutt_buffer_add (BUFFER * buf, const char *s, size_t len)
{
  size_t offset;

  if (buf->dptr + len + 1 > buf->data + buf->dsize) {
    offset = buf->dptr - buf->data;
    buf->dsize += len < 128 ? 128 : len + 1;
    mem_realloc ((void **) &buf->data, buf->dsize);
    buf->dptr = buf->data + offset;
  }
  memcpy (buf->dptr, s, len);
  buf->dptr += len;
  *(buf->dptr) = '\0';
}
