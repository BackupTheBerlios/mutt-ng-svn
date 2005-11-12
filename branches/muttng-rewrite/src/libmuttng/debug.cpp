/** @ingroup libmuttng */
/**
 * @file libmuttng/debug.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Abstract base class.
 */
#include <iostream>

#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdarg.h>

#include "core/io.h"
#include "core/buffer.h"
#include "core/intl.h"
#include "core/conv.h"
#include "core/str.h"

#include "debug.h"

Debug::Debug (const char* dir, const char* prefix, int u) {
  this->fp = NULL;
  this->level = 0;
  buffer_init ((&this->dir));
  buffer_init ((&this->prefix));
  if (dir)
    buffer_add_str (&this->dir, dir, -1);
  else
    buffer_add_ch (&this->dir, '.');
  if (this->dir.len && this->dir.str[this->dir.len-1] != '/')
    buffer_add_ch (&this->dir, '/');
  buffer_add_str (&this->prefix, prefix ? prefix : "libmuttng", -1);
  this->u = u;
}

Debug::~Debug (void) {
  buffer_free (&this->dir);
  buffer_free (&this->prefix);
  this->fp = NULL;
}

bool Debug::setLevel (int level) {
  bool rc = true;

  if (level >= 1 && level <= 5) {
    if (this->level < 1 || this->level > 5) {
      this->level = level;
      rc = start ();
    } else
      this->level = level;
    return (rc);
  }
  if (level <= 0 || level > 5) {
    rc = end ();
    this->level = 0;
    return (rc);
  }
  return (true);
}

bool Debug::start (void) {
  buffer_t fname;
  size_t len = 0, i = 0;
  struct stat st;

  if (this->level <= 0 || this->level > 5)
    return (false);

  buffer_init ((&fname));

  buffer_add_buffer (&fname, &this->dir);
  buffer_add_ch (&fname, '.');
  buffer_add_buffer (&fname, &this->prefix);
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

  if (!(this->fp = io_fopen (fname.str, "w", this->u < 0 ? 0077 : this->u))) {
    buffer_free (&fname);
    return (false);
  }
  buffer_free (&fname);

  DEBUGPRINT(this,1,(_("debug started at level %d for '%s'"),
                     this->level, NONULL (this->prefix.str)));
  return (true);
}

bool Debug::end (void) {
  bool rc = true;
  if (!this->fp)
    return (true);
  rc = fclose (this->fp) >= 0;
  this->fp = NULL;
  return (rc);
}

bool Debug::printIntro (const char* file, int line, const char* function,
                        int level) {
  if (level > this->level || !this->fp)
    return (false);
  fprintf (fp, "[%s:%s:%d:%s%s] ", NONULL (this->prefix.str),
           NONULL (file), line, NONULL (function), function ? "()" : "");
  return (true);
}

void Debug::printLine (const char* fmt, ...) {
  va_list ap;
  if (!this->fp)
    return;
  va_start (ap, fmt);
  vfprintf (fp, fmt, ap);
  va_end (ap);
  fputc ('\n', fp);
}
