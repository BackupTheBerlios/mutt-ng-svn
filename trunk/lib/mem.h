/*
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#ifndef _LIB_MEM_H
#define _LIB_MEM_H

void* _safe_malloc (size_t, int, const char*);
void* _safe_calloc (size_t, size_t, int, const char*);
void _safe_realloc (void*, size_t, int, const char*);
void _safe_free (void*);

#define safe_malloc(s) _safe_malloc(s,__LINE__,__FILE__)
#define safe_calloc(s,c) _safe_calloc(s,c,__LINE__,__FILE__)
#define safe_realloc(p,c) _safe_realloc(p,c,__LINE__,__FILE__)
#define FREE(x) _safe_free(x)

#endif /* !_LIB_MEM_H */
