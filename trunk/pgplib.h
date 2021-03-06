/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996,1997 Michael R. Elkins <me@mutt.org>
 * Copyright (C) 1999-2000 Thomas Roessler <roessler@does-not-exist.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#ifdef CRYPT_BACKEND_CLASSIC_PGP

#include "mutt_crypt.h"


typedef struct pgp_signature {
  struct pgp_signature *next;
  unsigned char sigtype;
  unsigned long sid1;
  unsigned long sid2;
} pgp_sig_t;

struct pgp_keyinfo {
  char *keyid;
  struct pgp_uid *address;
  int flags;
  short keylen;
  time_t gen_time;
  int numalg;
  const char *algorithm;
  struct pgp_keyinfo *parent;
  struct pgp_signature *sigs;
  struct pgp_keyinfo *next;

  short fp_len;                 /* length of fingerprint.
                                 * 20 for sha-1, 16 for md5.
                                 */
  unsigned char fingerprint[20];        /* large enough to hold SHA-1 and RIPEMD160
                                           hashes (20 bytes), MD5 hashes just use the
                                           first 16 bytes */
};

/* Note, that pgp_key_t is now pointer and declared in crypt.h */

typedef struct pgp_uid {
  char *addr;
  short trust;
  int flags;
  struct pgp_keyinfo *parent;
  struct pgp_uid *next;
  struct pgp_signature *sigs;
} pgp_uid_t;

enum pgp_version {
  PGP_V2,
  PGP_V3,
  PGP_GPG,
  PGP_UNKNOWN
};

/* prototypes */

const char *pgp_pkalgbytype (unsigned char);

pgp_key_t pgp_remove_key (pgp_key_t *, pgp_key_t);
pgp_uid_t *pgp_copy_uids (pgp_uid_t *, pgp_key_t);

short pgp_canencrypt (unsigned char);
short pgp_cansign (unsigned char);
short pgp_get_abilities (unsigned char);

void pgp_free_key (pgp_key_t * kpp);

#define pgp_new_keyinfo() mem_calloc (sizeof *((pgp_key_t)0), 1)

#endif /* CRYPT_BACKEND_CLASSIC_PGP */
