/*
 * Copyright notice from original mutt:
 * Copyright (C) 2001 Thomas Roessler <roessler@does-not-exist.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

/* 
 * Definitions for a rudimentary PGP packet parser which is shared
 * by mutt proper and the PGP public key ring lister.
 */

#ifndef _PGPPACKET_H
# define _PGPPACKET_H

enum packet_tags {
  PT_RES0 = 0,                  /* reserved */
  PT_ESK,                       /* Encrypted Session Key */
  PT_SIG,                       /* Signature Packet */
  PT_CESK,                      /* Conventionally Encrypted Session Key Packet */
  PT_OPS,                       /* One-Pass Signature Packet */
  PT_SECKEY,                    /* Secret Key Packet */
  PT_PUBKEY,                    /* Public Key Packet */
  PT_SUBSECKEY,                 /* Secret Subkey Packet */
  PT_COMPRESSED,                /* Compressed Data Packet */
  PT_SKE,                       /* Symmetrically Encrypted Data Packet */
  PT_MARKER,                    /* Marker Packet */
  PT_LITERAL,                   /* Literal Data Packet */
  PT_TRUST,                     /* Trust Packet */
  PT_NAME,                      /* Name Packet */
  PT_SUBKEY,                    /* Subkey Packet */
  PT_RES15,                     /* Reserved */
  PT_COMMENT                    /* Comment Packet */
};

unsigned char *pgp_read_packet (FILE * fp, size_t * len);
void pgp_release_packet (void);

#endif
