/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 * Copyright (C) 1999-2000 Thomas Roessler <roessler@does-not-exist.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

/*
 * This file used to contain some more functions, namely those
 * which are now in muttlib.c.  They have been removed, so we have
 * some of our "standard" functions in external programs, too.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include "io.h"

static int compare_stat (struct stat *osb, struct stat *nsb) {
  if (osb->st_dev != nsb->st_dev || osb->st_ino != nsb->st_ino ||
      osb->st_rdev != nsb->st_rdev) {
    return -1;
  }
  return 0;
}

int io_open (const char *path, int flags, int u) {
  struct stat osb, nsb;
  int fd;

  if (u > 0)
    umask (u);
  if ((fd = open (path, flags, 0666)) < 0)
    return fd;

  /* make sure the file is not symlink */
  if (lstat (path, &osb) < 0 || fstat (fd, &nsb) < 0 ||
      compare_stat (&osb, &nsb) == -1) {
    close (fd);
    return (-1);
  }

  return (fd);
}

/* when opening files for writing, make sure the file doesn't already exist
 * to avoid race conditions.
 */
FILE *io_fopen (const char *path, const char *mode, int u) {
  /* first set the current umask */
  if (mode[0] == 'w') {
    int fd;
    int flags = O_CREAT | O_EXCL;

#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif

    if (mode[1] == '+')
      flags |= O_RDWR;
    else
      flags |= O_WRONLY;

    if ((fd = io_open (path, flags, u)) < 0)
      return (NULL);

    return (fdopen (fd, mode));
  }
  else {
    if (u > 0)
      umask (u);
    return (fopen (path, mode));
  }
}
