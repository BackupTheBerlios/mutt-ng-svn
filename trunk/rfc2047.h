/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

char *mutt_choose_charset (const char *fromcode, const char *charsets,
                           char *u, size_t ulen, char **d, size_t * dlen);

void _rfc2047_encode_string (char **, int, int);
void rfc2047_encode_adrlist (ADDRESS *, const char *);

#define rfc2047_encode_string(a) _rfc2047_encode_string (a, 0, 32);

void rfc2047_decode (char **);
void rfc2047_decode_adrlist (ADDRESS *);
