/*
 * Copyright notice from original mutt:
 * Copyright (C) 2001 Thomas Roessler <roessler@does-not-exist.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

/* This module peeks at a PGP signature and figures out the hash
 * algorithm.
 */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "mutt.h"
#include "handler.h"
#include "pgp.h"
#include "pgppacket.h"
#include "mime.h"
#include "charset.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "lib/debug.h"

static struct {
  short id;
  const char *name;
} HashAlgorithms[] = {
  {
  1, "pgp-md5"}, {
  2, "pgp-sha1"}, {
  3, "pgp-ripemd160"}, {
  5, "pgp-md2"}, {
  6, "pgp-tiger192"}, {
  7, "pgp-haval-5-160"}, {
  8, "pgp-sha256"}, {
  9, "pgp-sha384"}, {
  10, "pgp-sha512"}, {
  -1, NULL}
};

static const char *pgp_hash_to_micalg (short id)
{
  int i;

  for (i = 0; HashAlgorithms[i].id >= 0; i++)
    if (HashAlgorithms[i].id == id)
      return HashAlgorithms[i].name;
  return "x-unknown";
}

static void pgp_dearmor (FILE * in, FILE * out)
{
  char line[HUGE_STRING];
  LOFF_T start;
  LOFF_T end;
  char *r;

  STATE state;

  memset (&state, 0, sizeof (STATE));
  state.fpin = in;
  state.fpout = out;

  /* find the beginning of ASCII armor */

  while ((r = fgets (line, sizeof (line), in)) != NULL) {
    if (!strncmp (line, "-----BEGIN", 10))
      break;
  }
  if (r == NULL) {
    debug_print (1, ("Can't find begin of ASCII armor.\n"));
    return;
  }

  /* skip the armor header */

  while ((r = fgets (line, sizeof (line), in)) != NULL) {
    SKIPWS (r);
    if (!*r)
      break;
  }
  if (r == NULL) {
    debug_print (1, ("Armor header doesn't end.\n"));
    return;
  }

  /* actual data starts here */
  start = ftello (in);

  /* find the checksum */

  while ((r = fgets (line, sizeof (line), in)) != NULL) {
    if (*line == '=' || !strncmp (line, "-----END", 8))
      break;
  }
  if (r == NULL) {
    debug_print (1, ("Can't find end of ASCII armor.\n"));
    return;
  }

  if ((end = ftello (in) - str_len (line)) < start) {
    debug_print (1, ("end < start???\n"));
    return;
  }

  if (fseeko (in, start, SEEK_SET) == -1) {
    debug_print (1, ("Can't seekto start.\n"));
    return;
  }

  mutt_decode_base64 (&state, end - start, 0, (iconv_t) - 1);
}

static short pgp_mic_from_packet (unsigned char *p, size_t len)
{
  /* is signature? */
  if ((p[0] & 0x3f) != PT_SIG) {
    debug_print (1, ("tag = %d, want %d.\n", p[0] & 0x3f, PT_SIG));
    return -1;
  }

  if (len >= 18 && p[1] == 3)
    /* version 3 signature */
    return (short) p[17];
  else if (len >= 5 && p[1] == 4)
    /* version 4 signature */
    return (short) p[4];
  else {
    debug_print (1, ("Bad signature packet.\n"));
    return -1;
  }
}

static short pgp_find_hash (const char *fname)
{
  FILE *in = NULL;
  FILE *out = NULL;

  char tempfile[_POSIX_PATH_MAX];

  unsigned char *p;
  size_t l;

  short rv = -1;

  mutt_mktemp (tempfile);
  if ((out = safe_fopen (tempfile, "w+")) == NULL) {
    mutt_perror (tempfile);
    goto bye;
  }
  unlink (tempfile);

  if ((in = fopen (fname, "r")) == NULL) {
    mutt_perror (fname);
    goto bye;
  }

  pgp_dearmor (in, out);
  rewind (out);

  if ((p = pgp_read_packet (out, &l)) != NULL) {
    rv = pgp_mic_from_packet (p, l);
  }
  else {
    debug_print (1, ("No packet.\n"));
  }

bye:

  safe_fclose (&in);
  safe_fclose (&out);
  pgp_release_packet ();
  return rv;
}

const char *pgp_micalg (const char *fname)
{
  return pgp_hash_to_micalg (pgp_find_hash (fname));
}
