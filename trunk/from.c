/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "mutt.h"

#include "lib/mem.h"
#include "lib/intl.h"
#include "lib/str.h"
#include "lib/debug.h"

#include <ctype.h>
#include <string.h>

static const char *next_word (const char *s)
{
  while (*s && !ISSPACE (*s))
    s++;
  SKIPWS (s);
  return s;
}

int mutt_check_month (const char *s)
{
  int i;

  for (i = 0; i < 12; i++)
    if (str_ncasecmp (s, Months[i], 3) == 0)
      return (i);
  return (-1);                  /* error */
}

static int is_day_name (const char *s)
{
  int i;

  if ((str_len (s) < 3) || !*(s + 3) || !ISSPACE (*(s + 3)))
    return 0;
  for (i = 0; i < 7; i++)
    if (str_ncasecmp (s, Weekdays[i], 3) == 0)
      return 1;
  return 0;
}

/*
 * A valid message separator looks like:
 *
 * From [ <return-path> ] <weekday> <month> <day> <time> [ <timezone> ] <year>
 */

int is_from (const char *s, char *path, size_t pathlen, time_t * tp)
{
  struct tm tm;
  int yr;

  if (path)
    *path = 0;

  if (str_ncmp ("From ", s, 5) != 0)
    return 0;

  s = next_word (s);            /* skip over the From part. */
  if (!*s)
    return 0;

  debug_print (3, ("parsing: %s\n", s));

  if (!is_day_name (s)) {
    const char *p;
    size_t len;
    short q = 0;

    for (p = s; *p && (q || !ISSPACE (*p)); p++) {
      if (*p == '\\') {
        if (*++p == '\0')
          return 0;
      }
      else if (*p == '"') {
        q = !q;
      }
    }

    if (q || !*p)
      return 0;

    if (path) {
      len = (size_t) (p - s);
      if (len + 1 > pathlen)
        len = pathlen - 1;
      memcpy (path, s, len);
      path[len] = 0;
      debug_print (3, ("got return path: %s\n", path));
    }

    s = p + 1;
    SKIPWS (s);
    if (!*s)
      return 0;

    if (!is_day_name (s)) {
      debug_print (1, ("expected weekday, got: %s\n", s));
      return 0;
    }
  }

  s = next_word (s);
  if (!*s)
    return 0;

  /* do a quick check to make sure that this isn't really the day of the week.
   * this could happen when receiving mail from a local user whose login name
   * is the same as a three-letter abbreviation of the day of the week.
   */
  if (is_day_name (s)) {
    s = next_word (s);
    if (!*s)
      return 0;
  }

  /* now we should be on the month. */
  if ((tm.tm_mon = mutt_check_month (s)) < 0)
    return 0;

  /* day */
  s = next_word (s);
  if (!*s)
    return 0;
  if (sscanf (s, "%d", &tm.tm_mday) != 1)
    return 0;

  /* time */
  s = next_word (s);
  if (!*s)
    return 0;

  /* Accept either HH:MM or HH:MM:SS */
  if (sscanf (s, "%d:%d:%d", &tm.tm_hour, &tm.tm_min, &tm.tm_sec) == 3);
  else if (sscanf (s, "%d:%d", &tm.tm_hour, &tm.tm_min) == 2)
    tm.tm_sec = 0;
  else
    return 0;

  s = next_word (s);
  if (!*s)
    return 0;

  /* timezone? */
  if (isalpha ((unsigned char) *s) || *s == '+' || *s == '-') {
    s = next_word (s);
    if (!*s)
      return 0;

    /*
     * some places have two timezone fields after the time, e.g.
     *      From xxxx@yyyyyyy.fr Wed Aug  2 00:39:12 MET DST 1995
     */
    if (isalpha ((unsigned char) *s)) {
      s = next_word (s);
      if (!*s)
        return 0;
    }
  }

  /* year */
  if (sscanf (s, "%d", &yr) != 1)
    return 0;
  tm.tm_year = yr > 1900 ? yr - 1900 : (yr < 70 ? yr + 100 : yr);

  debug_print (3, ("month=%d, day=%d, hr=%d, min=%d, sec=%d, yr=%d.\n",
           tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
           tm.tm_year));

  tm.tm_isdst = -1;

  if (tp)
    *tp = mutt_mktime (&tm, 0);
  return 1;
}
