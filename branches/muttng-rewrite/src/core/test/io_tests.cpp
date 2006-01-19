/** @ingroup core_unit */
/**
 * @file core/test/io_tests.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: I/O unit tests
 *
 * This file is published under the GNU General Public License.
 */
#include <iostream>

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

#include <unit++/unit++.h>
#include "io_tests.h"
#include "buffer.h"
#include "str.h"

using namespace unitpp;

/** number of tests for io_tempfile() */
#define TEMPFILE_TESTS  6

/** test cases for io_tempfile(). */
static struct {
  const char* dir;
  const char* name;
  const char* ext;
} TempfileTests[TEMPFILE_TESTS] = {
  { NULL,       "foo",          "bar"   },
  { "/tmp",     "bar",          NULL    },
  { NULL,       NULL,           "test"  },
  { NULL,       NULL,           NULL    },
  { NULL,       ".",            "."     },
  { "\0",       "\0",           "\0"    },
};

int io_tests::test_io_tempfile2 (const char* dir, const char* name,
                                 buffer_t* tempfile) {
  int fd = io_tempfile (dir, name, tempfile);
  struct stat st;

  if (fd < 0)
    return (0);
  close (fd);
  unlink (tempfile->str);
  /* where doing unit tests here so see if unlink() works */
  assert_eq ("stat(tmpfile) == -1", -1, stat (tempfile->str, &st));
  return (1);
}

void io_tests::test_io_tempfile (void) {
  buffer_t tempfile, name;
  int i = 0;

  buffer_init ((&tempfile));
  buffer_init ((&name));

  for (i = 0; i < TEMPFILE_TESTS; i++) {
    /* reset all buffers */
    buffer_shrink (&tempfile, 0);
    buffer_shrink (&name, 0);
    /* create name from name and extension */
    if (TempfileTests[i].name)
      buffer_add_str (&name, TempfileTests[i].name, -1);
    if (TempfileTests[i].ext) {
      buffer_add_ch (&name, '.');
      buffer_add_str (&name, TempfileTests[i].ext, -1);
    }
    /**
     * - 1st assertion:
     * we can create the tempfile
     */
    assert_eq ("test_io_tempfile()", 1,
               test_io_tempfile2 (TempfileTests[i].dir, name.str, &tempfile));
    /**
     * - 2nd assertion:
     * if we have a directory passed down to io_tempfile(),
     * the tempfile starts with it
     */
    if (TempfileTests[i].dir && *TempfileTests[i].dir) {
      assert_eq ("dir given but tempfile doesn't start with it", 0,
                 str_ncmp (tempfile.str, TempfileTests[i].dir,
                           str_len (TempfileTests[i].dir)));
    }
    /**
     * - 3rd assertion:
     * if we have a name passed down to io_tempfile(),
     * the tempfile contains the string
     */
    if (TempfileTests[i].name && *TempfileTests[i].name) {
      assert_eq ("name given but tempfile doesn't contain it", 1,
                 strstr (tempfile.str, TempfileTests[i].name) != NULL);
    }
    /**
     * - 4th assertion:
     * if we have an extension passed down to io_tempfile(),
     * the tempfile ends with it
     */
    if (TempfileTests[i].ext && *TempfileTests[i].ext) {
      size_t extlen = str_len (TempfileTests[i].ext);
      assert_eq ("ext given but tempfile doesn't end with it", 1,
                 tempfile.len > extlen &&
                 str_eq2 (tempfile.str + (tempfile.len - extlen),
                          TempfileTests[i].ext, extlen));
    }
  }
  buffer_free (&tempfile);
  buffer_free (&name);
}

void io_tests::test_io_open (void) {
  buffer_t tempfile, link, err;
  int fd = -1, l = 0;
  struct stat st;

  buffer_init ((&tempfile));
  buffer_init ((&link));
  buffer_init ((&err));

  /*
   * try to obtain secure tempfile
   * this is expected to succeed
   */
  if ((fd = io_tempfile (NULL, NULL, &tempfile)) < 0) {
    buffer_add_str (&err, "io_tempfile(", 12);
    buffer_add_buffer (&err, &tempfile);
    buffer_add_str (&err, ") = ", 4);
    buffer_add_snum (&err, fd, -1);
    assert_eq (err.str, fd >= 0, fd);
    goto out;
  }
  close (fd);

  /* link filename: 'tempfile.2' */
  buffer_add_buffer (&link, &tempfile);
  buffer_add_str (&link, ".2", 2);

  /*
   * try to create symlink to tempfile
   * this is expected to succeed
   */
  if ((l = symlink (tempfile.str, link.str)) == -1) {
    buffer_add_str (&err, "symlink(", 8);
    buffer_add_buffer (&err, &tempfile);
    buffer_add_ch (&err, ',');
    buffer_add_buffer (&err, &link);
    buffer_add_str (&err, ") = ", 4);
    buffer_add_snum (&err, l, -1);
    assert_eq (err.str, 0, l);
    goto out;
  }

  /*
   * now try to open link file read-only
   * this is expected to fail
   */
  if ((fd = io_open (link.str, O_RDONLY, -1)) >= 0) {
    buffer_add_str (&err, "io_open(", 8);
    buffer_add_buffer (&err, &link);
    buffer_add_str (&err, ") = ", 4);
    buffer_add_snum (&err, fd, -1);
    assert_eq (err.str, -1, fd);
    goto out;
  }

out:
  if (fd >= 0)
    close (fd);
  unlink (link.str);
  unlink (tempfile.str);
  /* where doing unit tests here so see if unlink() works */
  assert_eq ("lstat(link)", -1, lstat (link.str, &st));
  assert_eq ("stat(tempfile)", -1, stat (tempfile.str, &st));
  buffer_free (&tempfile);
  buffer_free (&link);
  /* just to make sure: fail if 1 != 0 to fail for message in err */
  if (err.len)
    assert_eq (err.str, 1, 0);
  buffer_free (&err);
}

void io_tests::test_io_readline() {
  buffer_t fname, line;
  unsigned int count = 0;
  int fd = -1, c = 0;

  buffer_init(&fname);
  buffer_init(&line);

  umask(0077);

  if ((fd = io_tempfile(NULL,NULL,&fname)) < 0) {
    buffer_free(&fname);
    assert_true("got opened tempfile",0);
    return;
  }
  write(fd,"line1\nline2 \\\nline2 \\\nline2\n\nline4",40);
  close(fd);
  FILE* fp = io_fopen(fname.str,"r",-1);
  assert_true(fname.str,fp!=NULL);
  while(++c <= 4) {
    count += io_readline(&line,fp);
    switch(c) {
    case 1:
      assert_true(line.str,buffer_equal1(&line,"line1",-1));
      assert_eq("at line 1 of input",1U,count);
      break;
    case 2:
      assert_true(line.str,buffer_equal1(&line,"line2 line2 line2",-1));
      assert_eq("at line 4 of input",4U,count);
      break;
    case 3:
      assert_true(line.str,buffer_equal1(&line,"",-1));
      assert_eq("at line 5 of input",5U,count);
      break;
    case 4:
      assert_true(line.str,buffer_equal1(&line,"line4",-1));
      assert_eq("at line 6 of input",6U,count);
      break;
    default:
      break;
    }
  }
  assert_eq("read 6 lines of input",6U,count);
  io_fclose(&fp);
  unlink(fname.str);
  buffer_free(&fname);
  buffer_free(&line);
}

io_tests::io_tests() : suite("io_tests") {
  add("io_tempfile()",testcase(this,"test_io_tempfile()",
                               &io_tests::test_io_tempfile));
  add("io_open()",testcase(this,"test_io_open()", &io_tests::test_io_open));
  add("io_readline()",testcase(this,"test_io_readline()", &io_tests::test_io_readline));
}
