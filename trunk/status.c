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
#include "mutt_menu.h"
#include "mutt_curses.h"
#include "sort.h"
#include "mapping.h"
#include "mx.h"
#include "buffy.h"

#include "lib/mem.h"
#include "lib/intl.h"
#include "lib/str.h"

#include <string.h>
#include <ctype.h>
#include <unistd.h>

static char *get_sort_str (char *buf, size_t buflen, int method)
{
  snprintf (buf, buflen, "%s%s%s",
            (method & SORT_REVERSE) ? "reverse-" : "",
            (method & SORT_LAST) ? "last-" : "",
            mutt_getnamebyvalue (method & SORT_MASK, SortMethods));
  return buf;
}

/* %b = number of incoming folders with unread messages [option]
 * %B = short mailbox path
 * %d = number of deleted messages [option]
 * %f = full mailbox path
 * %F = number of flagged messages [option]
 * %h = hostname
 * %l = length of mailbox (in bytes) [option]
 * %m = total number of messages [option]
 * %M = number of messages shown (virutal message count when limiting) [option]
 * %n = number of new messages [option]
 * %p = number of postponed messages [option]
 * %P = percent of way through index
 * %r = readonly/wontwrite/changed flag
 * %s = current sorting method ($sort)
 * %S = current aux sorting method ($sort_aux)
 * %t = # of tagged messages [option]
 * %v = Mutt-ng version 
 * %V = currently active limit pattern [option] */
static const char *status_format_str (char *buf, size_t buflen, char op,
                                      const char *src, const char *prefix,
                                      const char *ifstring,
                                      const char *elsestring,
                                      unsigned long data, format_flag flags)
{
  char fmt[SHORT_STRING], tmp[SHORT_STRING], *cp, *p;
  int count, optional = (flags & M_FORMAT_OPTIONAL);
  MUTTMENU *menu = (MUTTMENU *) data;

  *buf = 0;
  switch (op) {
  case 'b':
    if (!optional) {
      snprintf (fmt, sizeof (fmt), "%%%sd", prefix);
      snprintf (buf, buflen, fmt, buffy_check (0));
    }
    else if (!buffy_check (0))
      optional = 0;
    break;

  case 'B':
    snprintf (fmt, sizeof (fmt), "%%%ss", prefix);
#ifdef USE_COMPRESSED
    if (Context && Context->compressinfo && Context->realpath) {
      if ((p = strrchr (Context->realpath, '/')))
        strfcpy (tmp, p + 1, sizeof (tmp));
      else
        strfcpy (tmp, Context->realpath, sizeof (tmp));
    }
    else
#endif
    if (Context && Context->path) {
      if ((p = strrchr (Context->path, '/')))
        strfcpy (tmp, p + 1, sizeof (tmp));
      else
        strfcpy (tmp, Context->path, sizeof (tmp));
    }
    else
      strfcpy (tmp, _("no mailbox"), sizeof (tmp));
    snprintf (buf, buflen, fmt, tmp);
    break;

  case 'd':
    if (!optional) {
      snprintf (fmt, sizeof (fmt), "%%%sd", prefix);
      snprintf (buf, buflen, fmt, Context ? Context->deleted : 0);
    }
    else if (!Context || !Context->deleted)
      optional = 0;
    break;

  case 'h':
    snprintf (fmt, sizeof (fmt), "%%%ss", prefix);
    snprintf (buf, buflen, fmt, NONULL (Hostname));
    break;

  case 'f':
    snprintf (fmt, sizeof (fmt), "%%%ss", prefix);
#ifdef USE_COMPRESSED
    if (Context && Context->compressinfo && Context->realpath) {
      strfcpy (tmp, Context->realpath, sizeof (tmp));
      mutt_pretty_mailbox (tmp);
    }
    else
#endif
    if (Context && Context->path) {
      strfcpy (tmp, Context->path, sizeof (tmp));
      mutt_pretty_mailbox (tmp);
    }
    else
      strfcpy (tmp, _("(no mailbox)"), sizeof (tmp));
    snprintf (buf, buflen, fmt, tmp);
    break;

  case 'F':
    if (!optional) {
      snprintf (fmt, sizeof (fmt), "%%%sd", prefix);
      snprintf (buf, buflen, fmt, Context ? Context->flagged : 0);
    }
    else if (!Context || !Context->flagged)
      optional = 0;
    break;

  case 'l':
    if (!optional) {
      snprintf (fmt, sizeof (fmt), "%%%ss", prefix);
      mutt_pretty_size (tmp, sizeof (tmp), Context ? Context->size : 0);
      snprintf (buf, buflen, fmt, tmp);
    }
    else if (!Context || !Context->size)
      optional = 0;
    break;

  case 'L':
    if (!optional) {
      snprintf (fmt, sizeof (fmt), "%%%ss", prefix);
      mutt_pretty_size (tmp, sizeof (tmp), Context ? Context->vsize : 0);
      snprintf (buf, buflen, fmt, tmp);
    }
    else if (!Context || !Context->pattern)
      optional = 0;
    break;

  case 'm':
    if (!optional) {
      snprintf (fmt, sizeof (fmt), "%%%sd", prefix);
      snprintf (buf, buflen, fmt, Context ? Context->msgcount : 0);
    }
    else if (!Context || !Context->msgcount)
      optional = 0;
    break;

  case 'M':
    if (!optional) {
      snprintf (fmt, sizeof (fmt), "%%%sd", prefix);
      snprintf (buf, buflen, fmt, Context ? Context->vcount : 0);
    }
    else if (!Context || !Context->pattern)
      optional = 0;
    break;

  case 'n':
    if (!optional) {
      snprintf (fmt, sizeof (fmt), "%%%sd", prefix);
      snprintf (buf, buflen, fmt, Context ? Context->new : 0);
    }
    else if (!Context || Context->new <= 0)
      optional = 0;
    break;

  case 'o':
    if (!optional) {
      snprintf (fmt, sizeof (fmt), "%%%sd", prefix);
      snprintf (buf, buflen, fmt,
                Context ? Context->unread - Context->new : 0);
    }
    else if (!Context || !(Context->unread - Context->new))
      optional = 0;
    break;

  case 'p':
    count = mutt_num_postponed (0);
    if (!optional) {
      snprintf (fmt, sizeof (fmt), "%%%sd", prefix);
      snprintf (buf, buflen, fmt, count);
    }
    else if (!count)
      optional = 0;
    break;

  case 'P':
    if (menu->top + menu->pagelen >= menu->max)
      cp = menu->top ? "end" : "all";
    else {
      count = (100 * (menu->top + menu->pagelen)) / menu->max;
      snprintf (tmp, sizeof (tmp), "%d%%", count);
      cp = tmp;
    }
    snprintf (fmt, sizeof (fmt), "%%%ss", prefix);
    snprintf (buf, buflen, fmt, cp);
    break;

  case 'r':
    {
      int i = 0;

      if (Context) {
        i = option (OPTATTACHMSG) ? 3 : ((Context->readonly ||
                                          Context->dontwrite) ? 2 : (Context->
                                                                     changed
                                                                     || (
#ifdef USE_IMAP
                                                                             /* deleted doesn't necessarily mean changed in IMAP */
                                                                             Context->
                                                                             magic
                                                                             !=
                                                                             M_IMAP
                                                                             &&
#endif
                                                                             Context->
                                                                             deleted))
                                         ? 1 : 0);
      }

      if (!StChars)
        buf[0] = 0;
      else if (i >= str_len (StChars))
        buf[0] = StChars[0];
      else
        buf[0] = StChars[i];

      buf[1] = 0;
      break;
    }

  case 's':
    snprintf (fmt, sizeof (fmt), "%%%ss", prefix);
    snprintf (buf, buflen, fmt, get_sort_str (tmp, sizeof (tmp), Sort));
    break;

  case 'S':
    snprintf (fmt, sizeof (fmt), "%%%ss", prefix);
    snprintf (buf, buflen, fmt, get_sort_str (tmp, sizeof (tmp), SortAux));
    break;

  case 't':
    if (!optional) {
      snprintf (fmt, sizeof (fmt), "%%%sd", prefix);
      snprintf (buf, buflen, fmt, Context ? Context->tagged : 0);
    }
    else if (!Context || !Context->tagged)
      optional = 0;
    break;

  case 'u':
    if (!optional) {
      snprintf (fmt, sizeof (fmt), "%%%sd", prefix);
      snprintf (buf, buflen, fmt, Context ? Context->unread : 0);
    }
    else if (!Context || !Context->unread)
      optional = 0;
    break;

  case 'v':
    strncpy (buf, mutt_make_version (0), buflen);
    break;

  case 'V':
    if (!optional) {
      snprintf (fmt, sizeof (fmt), "%%%ss", prefix);
      snprintf (buf, buflen, fmt,
                (Context && Context->pattern) ? Context->pattern : "");
    }
    else if (!Context || !Context->pattern)
      optional = 0;
    break;

  case 0:
    *buf = 0;
    return (src);

  default:
    snprintf (buf, buflen, "%%%s%c", prefix, op);
    break;
  }

  if (optional)
    menu_status_line (buf, buflen, menu, ifstring);
  else if (flags & M_FORMAT_OPTIONAL)
    menu_status_line (buf, buflen, menu, elsestring);

  return (src);
}

void menu_status_line (char *buf, size_t buflen, MUTTMENU * menu,
                       const char *p)
{
  mutt_FormatString (buf, buflen, p, status_format_str, (unsigned long) menu,
                     0);
}
