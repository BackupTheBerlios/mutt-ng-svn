/*
 * Copyright notice from original mutt:
 * Copyright (C) 2004 Thomas Glanzmann <sithglan@stud.uni-erlangen.de>
 * Copyright (C) 2004 Tobias Werth <sitowert@stud.uni-erlangen.de>
 * Copyright (C) 2004 Brian Fundakowski Feldman <green@FreeBSD.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */
#ifndef _MUTT_HCACHE_H
#define _MUTT_HCACHE_H

#if USE_HCACHE
void *mutt_hcache_open(const char *path, const char *folder);
void mutt_hcache_close(void *db);
HEADER *mutt_hcache_restore(const unsigned char *d, HEADER **oh);
void *mutt_hcache_fetch(void *db, const char *filename,
                        size_t (*keylen)(const char *fn));
int mutt_hcache_store(void *db, const char *filename, HEADER *h,
                      unsigned long uid_validity,
                      size_t (*keylen)(const char *fn));
int mutt_hcache_delete(void *db, const char *filename,
                       size_t (*keylen)(const char *fn));
#endif /* USE_HCACHE */

#endif /* !_MUTT_HCACHE_H */
