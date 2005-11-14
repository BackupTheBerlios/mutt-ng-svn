/** @ingroup libmuttng */
/**
 * @file libmuttng/debug.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Debug implementation
 */
#include <iostream>

#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>

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
  end ();
  buffer_free (&this->dir);
  buffer_free (&this->prefix);
  this->fp = NULL;
}

bool Debug::setLevel (int level) {
  bool rc = true;

  if (level >= DEBUG_MIN+1 && level <= DEBUG_MAX) {
    if (this->level < DEBUG_MIN+1 || this->level > DEBUG_MAX) {
      this->level = level;
      rc = start ();
    } else
      this->level = level;
    return (rc);
  }
  if (level <= DEBUG_MIN || level > DEBUG_MAX) {
    rc = end ();
    this->level = 0;
    return (rc);
  }
  this->level = level;
  return (true);
}

bool Debug::start (void) {
  buffer_t fname;
  size_t len = 0, i = 0;
  struct stat st;
  time_t now = 0;

  if (this->level <= DEBUG_MIN || this->level > DEBUG_MAX)
    return (false);

  buffer_init ((&fname));

  buffer_add_buffer (&fname, &this->dir);
  buffer_add_ch (&fname, '.');
  buffer_add_buffer (&fname, &this->prefix);
  buffer_add_ch (&fname, '.');
  buffer_add_num (&fname, getpid (), -1);
  buffer_add_ch (&fname, '.');
  len = fname.len;

  while (i++ < MAX_DBG_FILES) {
    buffer_shrink (&fname, len);
    buffer_add_num (&fname, i, -1);
    buffer_add_str (&fname, ".log", 4);
    if (stat (fname.str, &st) == -1)
      break;
  }

  if (i == MAX_DBG_FILES+1) {
    buffer_free (&fname);
    return (false);
  }

  if (!(this->fp = io_fopen (fname.str, "w", this->u))) {
    buffer_free (&fname);
    return (false);
  }
  buffer_free (&fname);

  now = time (NULL);

  if (printIntro (__FILE__, __LINE__, NULL, 1))
    printLine (_("debug %sstarted at level %d for %s at %s"),
               i == 1 ? "" : "re-", this->level, NONULL (this->prefix.str),
               asctime (localtime (&now)));
  return (true);
}

bool Debug::end (void) {
  bool rc = true;
  time_t now = 0;

  if (!this->fp)
    return (true);

  now = time (NULL);
  if (printIntro (__FILE__, __LINE__, NULL, 1))
    printLine (_("debug finished at level %d for %s at %s"),
               this->level, NONULL (this->prefix.str),
               asctime (localtime (&now)));

  rc = fclose (this->fp) >= 0;
  this->fp = NULL;
  return (rc);
}

bool Debug::printIntro (const char* file, int line, const char* function,
                        int level) {
  if (level > this->level || !this->fp)
    return (false);
  fprintf (fp, "%d [%s:%s:%d%s%s%s] ", level,
           NONULL (this->prefix.str), NONULL (file), line, 
           function ? ":" : "", NONULL (function), function ? "()" : "");
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
