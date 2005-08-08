/*
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */
#ifndef _LIB_MEM_H
#define _LIB_MEM_H

#include <sys/types.h>

void* _mem_malloc (size_t, int, const char*);
void* _mem_calloc (size_t, size_t, int, const char*);
void _mem_realloc (void*, size_t, int, const char*);
void _mem_free (void*);

#define mem_malloc(s) _mem_malloc(s,__LINE__,__FILE__)
#define mem_calloc(s,c) _mem_calloc(s,c,__LINE__,__FILE__)
#define mem_realloc(p,c) _mem_realloc(p,c,__LINE__,__FILE__)
#define mem_free(x) _mem_free(x)

#endif /* !_LIB_MEM_H */
