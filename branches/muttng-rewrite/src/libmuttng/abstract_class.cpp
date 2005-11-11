/** @ingroup libmuttng */
/**
 * @file libmuttng/abstract_class.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Abstract base class.
 */
#include <iostream>

#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "core/io.h"
#include "core/buffer.h"
#include "core/intl.h"
#include "core/conv.h"
#include "core/str.h"

#include "abstract_class.h"

/** Pointer for debug file */
static FILE* FP = NULL;
/** Directory debugfile is stored in */
static buffer_t Dir = { NULL, 0, 0 };
/** Prefix */
static buffer_t Prefix = { NULL, 0, 0 };
/** Current debug level */
static int Level = 0;

Debug::Debug (void) {}
Debug::~Debug (void) {}

void Debug::init (const char* dir, const char* prefix) {
  FP = NULL;
  buffer_init ((&Dir));
  buffer_init ((&Prefix));
  if (dir)
    buffer_add_str (&Dir, dir, -1);
  else
    buffer_add_ch (&Dir, '.');
  if (Dir.len && Dir.str[Dir.len-1] != '/')
      buffer_add_ch (&Dir, '/');
  buffer_add_str (&Prefix, prefix ? prefix : "libmuttng", -1);
}

void Debug::setLevel (int level) {
  if (level >= 1 && level <= 5)
    Level = level;
  else if (level <= 0)
    Debug::end ();
}

/**
 * @bug When config works, pass $umask in here for io_fopen().
 */
bool Debug::start (void) {
  buffer_t fname;
  size_t len = 0, i = 0;
  struct stat st;

  if (Level <= 0 || Level > 5)
    return (false);

  buffer_init ((&fname));

  buffer_add_buffer (&fname, &Dir);
  buffer_add_ch (&fname, '.');
  buffer_add_buffer (&fname, &Prefix);
  buffer_add_ch (&fname, '.');
  buffer_add_num (&fname, getpid (), -1);
  buffer_add_ch (&fname, '.');
  len = fname.len;

  while (i++ < 5) {
    buffer_shrink (&fname, len);
    buffer_add_num (&fname, i, -1);
    if (stat (fname.str, &st) == -1)
      break;
  }

  if (i == 5+1) {
    buffer_free (&fname);
    return (false);
  }

  buffer_add_str (&fname, ".log", 4);

  if (!(FP = io_fopen (fname.str, "w", 0077))) {
    buffer_free (&fname);
    return (false);
  }

  buffer_shrink (&fname, 0);
  buffer_add_str (&fname, _("debug started at level "), -1);
  buffer_add_num (&fname, Level, -1);
  buffer_add_ch (&fname, '\n');
  DEBUGPRINT(1,Prefix.str);
  print (1, ":\n");
  DEBUGPRINT(1,fname.str);

  return (true);
}

bool Debug::end (void) {
  if (!FP)
    return (true);
  return (fclose (FP) >= 0);
}

void Debug::intro (int level, const char* file, int line, const char* func) {
  buffer_t str;

  if (!FP || Level < level)
    return;
  buffer_init ((&str));
  buffer_add_ch (&str, '[');
  buffer_add_str (&str, file, -1); buffer_add_ch (&str, ':');
  buffer_add_num (&str, line, -1);
  if (func) {
    buffer_add_ch (&str, ':');
    buffer_add_str (&str, func, -1);
    buffer_add_str (&str, "()", 2);
  }
  buffer_add_str (&str, "] ", 2);
  write (fileno (FP), str.str, str.len);
  buffer_free (&str);
}

void Debug::print (int level, const char* msg) {
  if (FP && Level >= level)
    write (fileno (FP), msg, str_len (msg));
}

void Debug::print (int level, int num) {
  char buf[NUMBUF];
  Debug::print (level, conv_itoa (buf, num, -1));
}
