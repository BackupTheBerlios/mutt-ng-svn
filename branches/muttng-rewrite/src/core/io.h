/**
 * @ingroup core
 * @addtogroup core_io I/O routines
 * @{
 */
/**
 * @file core/io.h
 * @author Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 * @author Copyright (C) 1999-2000 Thomas Roessler <roessler@does-not-exist.org>
 * @brief Interface: I/O routines
 */
#ifndef MUTTNG_CORE_IO_H
#define MUTTNG_CORE_IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include "buffer.h"

/**
 * Like @c open(2) but fails for symlinks.
 * @param path File to open.
 * @param flags Flags to pass to @c open(2).
 * @param u If positive, pass to @c umask(2) first.
 * @return Open file descriptor or @c -1 in case of error.
 * @test io_tests::test_io_open().
 */
int io_open (const char* path, int flags, int u);

/**
 * Open file for reading. If filename ends with a "|" symbol,
 * don't interpret path as filename but command to execute and
 * read from.
 * @param path Filename or command.
 * @param pid Pointer to pid_t which is modified in case a command is found.
 * @return File pointer opened for reading.
 */
FILE* io_open_read(const char* path, pid_t* pid);

/**
 * Like @c fopen(3) but tries to prevent symlink attacks.
 * @param path File to open.
 * @param mode Mode to pass to @c fopen(3).
 * @param u If positive, pass via io_open() to @c umask(2) first.
 * @return File pointer or @c NULL in case of error.
 */
FILE* io_fopen (const char *path, const char *mode, int u);

/**
 * Obtain opened temporary file.
 * This routine has a series of features:
 *   -# If @c dir is given, the temporary file will be created
 *      under it. If none given, @c /tmp will be used.
 *   -# If @c name is given, the name will be used as a template
 *      for the filename. If none given, @c muttng will be used.
 *      Also, if the name contains a period, it's believed the
 *      name has an extension to be kept since several applications
 *      seem to rely on filenames. For example, for a template of
 *      @c foo.html one way want to have a temporary file for it,
 *      e.g. @c foo.XXXXXXXX.html to keep the @c .html suffix.
 *      <b>BIG FAT WARNING: IN THIS CASE AND ONLY THIS CASE,
 *      @c mktemp() HAS TO BE USED AS BOTH, @c mktemp() AND
 *      @c mkstemp(), EXPECT THE PLACEHOLDERS AT THE END OF THE PASSED
 *      NAME WHICH DOESN'T WORK WHEN WANTING TO KEEP THE EXTENSION.
 *      SO TRY TO AVOID PASSING IN NAMES WITH EXTENSIONS TO ENSURE USE
 *      OF @c mkstemp().</b>
 * @param dir Directory or @c NULL.
 * @param name If not @c NULL, take this as suggestion for filename.
 * @param tempfile Buffer where filename will be stored. It is
 *                 shrinked to zero length using @c buffer_shrink()
 *                 so any data will be lost.
 * @return Opon success, the open file descriptor is returned and
 *         @c -1 otherwise.
 * @bug This <i>may</i> use @c mktemp().
 * @test io_tests::test_io_tempfile().
 */
int io_tempfile (const char* dir, const char* name, buffer_t* tempfile);

/**
 * Sanity wrapper for fclose().
 * @param fp File pointer.
 * @return result of fclose().
 */
int io_fclose(FILE** fp);

/**
 * Read a single line from input into buffer. If the line's end is
 * escaped, i.e. ends in \\, keep reading until either end of input
 * or non-escaped endline.
 * @param dst Destination buffer.
 * @param fp Where to read from.
 * @return Number of lines read, i.e. 0 in case of error and positive
 * otherwise.
 */
unsigned int io_readline(buffer_t* dst, FILE* fp);

#ifdef __cplusplus
}
#endif

#endif

/** @} */
