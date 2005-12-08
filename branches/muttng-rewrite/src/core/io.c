/** @ingroup core_io */
/**
 * @file core/io.c
 * @author Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 * @author Copyright (C) 1999-2000 Thomas Roessler <roessler@does-not-exist.org>
 * @brief Implementation: I/O routines
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "io.h"

/**
 * Compare two <tt>struct stat</tt>s.
 * @param osb 1st structure.
 * @param nsb 2nd structure.
 * @return
 *   - 0 if equal
 *   - negative otherwise
 */
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

int io_tempfile (const char* dir, const char* name, buffer_t* tempfile) {
  const char* tmpdir = (dir && *dir ? dir : "/tmp");
  const char* name2 = (name && *name ? name : "muttng");
  char x[8] = "XXXXXXXX";
  char* period = NULL;

  if (!tempfile)
    return (-1);

  /* append 'tempdir/' */
  buffer_shrink (tempfile, 0);
  buffer_add_str (tempfile, tmpdir, -1);
  buffer_add_ch (tempfile, '/');

  if ((period = strrchr (name2, '.'))) {
    /*
     * if we were given an extension, append name
     * upto period, run mktemp() since Xs must be _trailing_,
     * add extension and try to open file
     */
    buffer_add_str (tempfile, name2, period-name2);
    /* add '.XXXXXXXX' */
    buffer_add_ch (tempfile, '.');
    buffer_add_str (tempfile, x, 8);
    mktemp (tempfile->str);
    /* if name given contain '.', append extension */
    buffer_add_str (tempfile, period, -1);
    return (io_open (tempfile->str, O_CREAT | O_EXCL |
#ifdef O_NOFOLLOW
                                   O_NOFOLLOW |
#endif
                                   O_RDWR, 0600));
  }

  /* no extension in name: add '$name.XXXXXXXX' */
  buffer_add_str (tempfile, name2, -1);
  buffer_add_ch (tempfile, '.');
  buffer_add_str (tempfile, x, 8);

  return (mkstemp (tempfile->str));
}
