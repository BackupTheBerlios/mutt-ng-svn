/**
 * @ingroup core
 * @addtogroup core_mem Memory handling
 * @{
 */
/**
 * @file core/mem.h
 * @author Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 * @author Copyright (C) 1999-2000 Thomas Roessler <roessler@does-not-exist.org>
 * @brief Interface: Sanity memory handling
 */
#ifndef MUTTNG_CORE_MEM_H
#define MUTTNG_CORE_MEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>

/**
 * @c malloc() wrapper.
 * @param nmemb Allocate this many bytes.
 * @param line Source line of caller.
 * @param fname Source file of caller.
 * @return memory.
 * @sa mem_malloc().
 */
void* _mem_malloc (size_t nmemb, int line, const char* fname);

/**
 * @c calloc() wrapper.
 * @param nmemb Allocate this many bytes.
 * @param size Allocate this many members of nmemb size.
 * @param line Source line of caller.
 * @param fname Source file of caller.
 * @return memory.
 * @sa mem_calloc().
 */
void* _mem_calloc (size_t nmemb, size_t size, int line, const char* fname);

/**
 * @c realloc() wrapper.
 * @param ptr Pointer of storage. <b>NOTE: THIS IS DEREFERENCE SO PASS
 *                                <tt>VOID**</tt> INSTEAD OF
 *                                <tt>VOID*</tt></b>.
 * @param siz To which byte size to reallocate memory.
 * @param line Source line of caller.
 * @param fname Source file of caller.
 * @sa mem_realloc().
 */
void _mem_realloc (void* ptr, size_t siz, int line, const char* fname);

/**
 * @c free() wrapper.
 * @param ptr Pointer of storage. <b>NOTE: THIS IS DEREFERENCE SO PASS
 *                                <tt>VOID**</tt> INSTEAD OF
 *                                <tt>VOID*</tt></b>.
 * @sa mem_free().
 */
void _mem_free (void* ptr);

/**
 * Wrapper for _mem_malloc() to get origin via preprocessor.
 * @param s byte size.
 * @return memory.
 */
#define mem_malloc(s) _mem_malloc(s,__LINE__,__FILE__)

/**
 * Wrapper for _mem_calloc() to get origin via preprocessor.
 * @param s member size
 * @param c member count.
 * @return memory.
 */
#define mem_calloc(s,c) _mem_calloc(s,c,__LINE__,__FILE__)

/**
 * Wrapper for _mem_malloc() to get origin via preprocessor.
 * @param p pointer.
 * @param c count.
 */
#define mem_realloc(p,c) _mem_realloc(p,c,__LINE__,__FILE__)

/**
 * Wrapper for _mem_malloc() to get origin via preprocessor.
 * @param x pointer.
 */
#define mem_free(x) _mem_free(x)

#ifdef __cplusplus
}
#endif

#endif /* !MUTTNG_CORE_MEM_H */

/** @} */
