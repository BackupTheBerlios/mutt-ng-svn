/**
 * @ingroup core
 */
/**
 * @file core/mem.h
 * @author Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 * @author Copyright (C) 1999-2000 Thomas Roessler <roessler@does-not-exist.org>
 * @brief Sanity memory handling interface
 */
#ifndef MUTTNG_CORE_MEM_H
#define MUTTNG_CORE_MEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>

void* _mem_malloc (size_t, int, const char*);
void* _mem_calloc (size_t, size_t, int, const char*);
void _mem_realloc (void*, size_t, int, const char*);
void _mem_free (void*);

#define mem_malloc(s) _mem_malloc(s,__LINE__,__FILE__)
#define mem_calloc(s,c) _mem_calloc(s,c,__LINE__,__FILE__)
#define mem_realloc(p,c) _mem_realloc(p,c,__LINE__,__FILE__)
#define mem_free(x) _mem_free(x)

#ifdef __cplusplus
}
#endif

#endif /* !MUTTNG_CORE_MEM_H */
