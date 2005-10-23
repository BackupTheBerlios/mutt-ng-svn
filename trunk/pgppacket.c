/*
 * Copyright notice from original mutt:
 * Copyright (C) 2001 Thomas Roessler <roessler@does-not-exist.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "sha1.h"
#include "lib.h"
#include "pgplib.h"
#include "pgppacket.h"

#include "lib/mem.h"

#define CHUNKSIZE 1024

static unsigned char *pbuf = NULL;
static size_t plen = 0;

static int read_material (size_t material, size_t * used, FILE * fp)
{
  if (*used + material >= plen) {
    unsigned char *p;
    size_t nplen;

    nplen = *used + material + CHUNKSIZE;

    if (!(p = realloc (pbuf, nplen))) { /* __MEM_CHECKED__ */
      perror ("realloc");
      return -1;
    }
    plen = nplen;
    pbuf = p;
  }

  if (fread (pbuf + *used, 1, material, fp) < material) {
    perror ("fread");
    return -1;
  }

  *used += material;
  return 0;
}

unsigned char *pgp_read_packet (FILE * fp, size_t * len)
{
  size_t used = 0;
  LOFF_T startpos;
  unsigned char ctb;
  unsigned char b;
  size_t material;

  startpos = ftello (fp);

  if (!plen) {
    plen = CHUNKSIZE;
    pbuf = mem_malloc (plen);
  }

  if (fread (&ctb, 1, 1, fp) < 1) {
    if (!feof (fp))
      perror ("fread");
    goto bail;
  }

  if (!(ctb & 0x80)) {
    goto bail;
  }

  if (ctb & 0x40) {             /* handle PGP 5.0 packets. */
    int partial = 0;

    pbuf[0] = ctb;
    used++;

    do {
      if (fread (&b, 1, 1, fp) < 1) {
        perror ("fread");
        goto bail;
      }

      if (b < 192) {
        material = b;
        partial = 0;
        /* material -= 1; */
      }
      else if (192 <= b && b <= 223) {
        material = (b - 192) * 256;
        if (fread (&b, 1, 1, fp) < 1) {
          perror ("fread");
          goto bail;
        }
        material += b + 192;
        partial = 0;
        /* material -= 2; */
      }
      else if (b < 255) {
        material = 1 << (b & 0x1f);
        partial = 1;
        /* material -= 1; */
      }
      else
        /* b == 255 */
      {
        unsigned char buf[4];

        if (fread (buf, 4, 1, fp) < 1) {
          perror ("fread");
          goto bail;
        }
        /*assert( sizeof(material) >= 4 ); */
        material = buf[0] << 24;
        material |= buf[1] << 16;
        material |= buf[2] << 8;
        material |= buf[3];
        partial = 0;
        /* material -= 5; */
      }

      if (read_material (material, &used, fp) == -1)
        goto bail;

    }
    while (partial);
  }
  else
    /* Old-Style PGP */
  {
    int bytes = 0;

    pbuf[0] = 0x80 | ((ctb >> 2) & 0x0f);
    used++;

    switch (ctb & 0x03) {
    case 0:
      {
        if (fread (&b, 1, 1, fp) < 1) {
          perror ("fread");
          goto bail;
        }

        material = b;
        break;
      }

    case 1:
      bytes = 2;

    case 2:
      {
        int i;

        if (!bytes)
          bytes = 4;

        material = 0;

        for (i = 0; i < bytes; i++) {
          if (fread (&b, 1, 1, fp) < 1) {
            perror ("fread");
            goto bail;
          }

          material = (material << 8) + b;
        }
        break;
      }

    default:
      goto bail;
    }

    if (read_material (material, &used, fp) == -1)
      goto bail;
  }

  if (len)
    *len = used;

  return pbuf;

bail:

  fseeko (fp, startpos, SEEK_SET);
  return NULL;
}

void pgp_release_packet (void)
{
  plen = 0;
  mem_free (&pbuf);
}
