/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2002 Michael R. Elkins <me@mutt.org>
 *
 * Parts were written/modified by:
 * Rocco Rutte <pdmef@cs.tu-berlin.de>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "mutt.h"
#include "buffer.h"
#include "ascii.h"
#include "mapping.h"
#include "mutt_curses.h"
#include "history.h"
#include "keymap.h"
#include "mbyte.h"
#include "charset.h"
#include "thread.h"
#include "mutt_crypt.h"
#include "mutt_idna.h"

#if defined(USE_SSL) || defined(USE_GNUTLS)
#include "mutt_ssl.h"
#endif

#if defined (USE_LIBESMTP) && (defined (USE_SSL) || defined (USE_GNUTLS))
#include "mutt_libesmtp.h"
#endif

#include "mx.h"
#include "init.h"

#include "lib/mem.h"
#include "lib/intl.h"
#include "lib/str.h"
#include "lib/rx.h"
#include "lib/list.h"
#include "lib/debug.h"

#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/utsname.h>
#include <errno.h>
#include <sys/wait.h>

#define CHECK_PAGER \
  if ((CurrentMenu == MENU_PAGER) && \
      (!option || (option->flags & R_RESORT))) \
  { \
    snprintf (err->data, err->dsize, \
              _("Not available in this menu.")); \
    return (-1); \
  } else

/*
 * prototypes
 */
static const struct mapping_t* get_sortmap (struct option_t* option);
static int parse_sort (struct option_t* dst, const char *s,
                       const struct mapping_t *map,
                       char* errbuf, size_t errlen);

static HASH* ConfigOptions = NULL;

/* for synonym warning reports: synonym found during parsing */
typedef struct {
  char* f;              /* file */
  int l;                /* line */
  struct option_t* n;   /* new */
  struct option_t* o;   /* old */
} syn_t;

/* for synonym warning reports: list of synonyms found */
static list2_t* Synonyms;
/* for synonym warning reports: current rc file */
static const char* CurRCFile = NULL;
/* for synonym warning reports: current rc line */
static int CurRCLine = 0;

/* prototypes for checking for special vars */
static int check_dsn_return (const char* option, unsigned long val,
                             char* errbuf, size_t errlen);
static int check_dsn_notify (const char* option, unsigned long val,
                             char* errbuf, size_t errlen);
static int check_history    (const char* option, unsigned long val,
                             char* errbuf, size_t errlen);
/* this checks that numbers are >= 0 */
static int check_num        (const char* option, unsigned long val,
                             char* errbuf, size_t errlen);
#ifdef DEBUG
static int check_debug      (const char* option, unsigned long val,
                             char* errbuf, size_t errlen);
#endif

/* use this to check only */
static int check_special (const char* option, unsigned long val,
                          char* errbuf, size_t errlen);

/* variable <-> sanity check function mappings
 * when changing these, make sure the proper _from_string handler
 * does this checking!
 */
static struct {
  const char* name;
  int (*check) (const char* option, unsigned long val,
                char* errbuf, size_t errlen);
} SpecialVars[] = {
  { "dsn_notify",               check_dsn_notify },
  { "dsn_return",               check_dsn_return },
#if defined (USE_LIBESMTP) && (defined (USE_SSL) || defined (USE_GNUTLS))
  { "smtp_use_tls",             mutt_libesmtp_check_usetls },
#endif
  { "history",                  check_history },
  { "pager_index_lines",        check_num },
#ifdef DEBUG
  { "debug_level",              check_debug },
#endif
  /* last */
  { NULL,         NULL }
};

/* protos for config type handles: convert value to string */
static void bool_to_string  (char* dst, size_t dstlen, struct option_t* option);
static void num_to_string   (char* dst, size_t dstlen, struct option_t* option);
static void str_to_string   (char* dst, size_t dstlen, struct option_t* option);
static void quad_to_string  (char* dst, size_t dstlen, struct option_t* option);
static void sort_to_string  (char* dst, size_t dstlen, struct option_t* option);
static void rx_to_string    (char* dst, size_t dstlen, struct option_t* option);
static void magic_to_string (char* dst, size_t dstlen, struct option_t* option);
static void addr_to_string  (char* dst, size_t dstlen, struct option_t* option);
static void user_to_string  (char* dst, size_t dstlen, struct option_t* option);
static void sys_to_string   (char* dst, size_t dstlen, struct option_t* option);

/* protos for config type handles: convert to value from string */
static int bool_from_string  (struct option_t* dst, const char* val,
                              char* errbuf, size_t errlen);
static int num_from_string   (struct option_t* dst, const char* val,
                              char* errbuf, size_t errlen);
static int str_from_string   (struct option_t* dst, const char* val,
                              char* errbuf, size_t errlen);
static int path_from_string  (struct option_t* dst, const char* val,
                              char* errbuf, size_t errlen);
static int quad_from_string  (struct option_t* dst, const char* val,
                              char* errbuf, size_t errlen);
static int sort_from_string  (struct option_t* dst, const char* val,
                              char* errbuf, size_t errlen);
static int rx_from_string    (struct option_t* dst, const char* val,
                              char* errbuf, size_t errlen);
static int magic_from_string (struct option_t* dst, const char* val,
                              char* errbuf, size_t errlen);
static int addr_from_string  (struct option_t* dst, const char* val,
                              char* errbuf, size_t errlen);
static int user_from_string  (struct option_t* dst, const char* val,
                              char* errbuf, size_t errlen);

static struct {
  unsigned short type;
  void (*opt_to_string) (char* dst, size_t dstlen, struct option_t* option);
  int (*opt_from_string) (struct option_t* dst, const char* val,
                          char* errbuf, size_t errlen);
} FuncTable[] = {
  { 0,          NULL,             NULL }, /* there's no DT_ type with 0 */
  { DT_BOOL,    bool_to_string,   bool_from_string },
  { DT_NUM,     num_to_string,    num_from_string },
  { DT_STR,     str_to_string,    str_from_string },
  { DT_PATH,    str_to_string,    path_from_string },
  { DT_QUAD,    quad_to_string,   quad_from_string },
  { DT_SORT,    sort_to_string,   sort_from_string },
  { DT_RX,      rx_to_string,     rx_from_string },
  { DT_MAGIC,   magic_to_string,  magic_from_string },
  /* synonyms should be resolved already so we don't need this
   * but must define it as DT_ is used for indexing */
  { DT_SYN,     NULL,             NULL },
  { DT_ADDR,    addr_to_string,   addr_from_string },
  { DT_USER,    user_to_string,   user_from_string },
  { DT_SYS,     sys_to_string,    NULL },
};

static void bool_to_string (char* dst, size_t dstlen,
                            struct option_t* option) {
  snprintf (dst, dstlen, "%s=%s", option->option,
            option (option->data) ? "yes" : "no");
}

static int bool_from_string (struct option_t* dst, const char* val,
                             char* errbuf, size_t errlen) {
  int flag = -1;

  if (!dst)
    return (0);
  if (ascii_strncasecmp (val, "yes", 3) == 0)
    flag = 1;
  else if (ascii_strncasecmp (val, "no", 2) == 0)
    flag = 0;

  if (flag < 0)
    return (0);
  if (flag)
    set_option (dst->data);
  else
    unset_option (dst->data);
  return (1);
}

static void num_to_string (char* dst, size_t dstlen,
                           struct option_t* option) {
  /* XXX puke */
  const char* fmt = (str_cmp (option->option, "umask") == 0) ? 
                    "%s=%04o" : "%s=%d";
  snprintf (dst, dstlen, fmt, option->option,
            *((short*) option->data));
}

static int num_from_string (struct option_t* dst, const char* val,
                            char* errbuf, size_t errlen) {
  int num = 0, old = 0;
  char* t = NULL;

  if (!dst)
    return (0);

  num = strtol (val, &t, 0);

  if (!*val || *t || (short) num != num) {
    if (errbuf) {
      snprintf (errbuf, errlen, _("'%s' is invalid for $%s"),
                val, dst->option);
    }
    return (0);
  }

  /* just temporarily accept new val so that check_special for
   * $history already has it when doing history's init() */
  old = *((short*) dst->data);
  *((short*) dst->data) = (short) num;

  if (!check_special (dst->option, (unsigned long) num, errbuf, errlen)) {
    *((short*) dst->data) = old;
    return (0);
  }

  return (1);
}

static void str_to_string (char* dst, size_t dstlen,
                           struct option_t* option) {
  snprintf (dst, dstlen, "%s=\"%s\"", option->option,
            NONULL (*((char**) option->data)));
}

static void user_to_string (char* dst, size_t dstlen,
                            struct option_t* option) {
  snprintf (dst, dstlen, "%s=\"%s\"", option->option,
            NONULL (((char*) option->data)));
}

static void sys_to_string (char* dst, size_t dstlen,
                           struct option_t* option) {
  char* val = NULL, *t = NULL;
  int clean = 0;

  /* get some $muttng_ values dynamically */
  if (ascii_strcmp ("muttng_pwd", option->option) == 0) {
    val = mem_malloc (_POSIX_PATH_MAX);
    val = getcwd (val, _POSIX_PATH_MAX-1);
    clean = 1;
  } else if (ascii_strcmp ("muttng_folder_path", option->option) == 0 &&
             CurrentFolder && *CurrentFolder) {
    val = CurrentFolder;
  } else if (ascii_strcmp ("muttng_folder_name", option->option) == 0 &&
             CurrentFolder && *CurrentFolder) {

    size_t Maildirlength = str_len (Maildir);

    /*
     * if name starts with $folder, just strip it to keep hierarchy
     * $folder=imap://host, path=imap://host/inbox/b -> inbox/b
     */
    if (Maildirlength > 0 && str_ncmp (CurrentFolder, Maildir, 
                                      Maildirlength) == 0 && 
       str_len (CurrentFolder) > Maildirlength) {
     val = CurrentFolder + Maildirlength;
     if (Maildir[strlen(Maildir)-1]!='/')
       val += 1;
     /* if not $folder, just use everything after last / */
    } else if ((t = strrchr (CurrentFolder, '/')) != NULL)
      val = t+1;
    /* default: use as-is */
    else
      val = CurrentFolder;

  } else
    val = option->init;

  snprintf (dst, dstlen, "%s=\"%s\"", option->option, NONULL (val));
  if (clean)
    mem_free (&val);
}

static int path_from_string (struct option_t* dst, const char* val,
                             char* errbuf, size_t errlen) {
  char path[_POSIX_PATH_MAX];

  if (!dst)
    return (0);

  if (!val || !*val) {
    mem_free ((char**) dst->data);
    return (1);
  }

  path[0] = '\0';
  strfcpy (path, val, sizeof (path));
  mutt_expand_path (path, sizeof (path));
  str_replace ((char **) dst->data, path);
  return (1);
}

static int str_from_string (struct option_t* dst, const char* val,
                            char* errbuf, size_t errlen) {
  if (!dst)
    return (0);

  if (!check_special (dst->option, (unsigned long) val, errbuf, errlen))
    return (0);

  str_replace ((char**) dst->data, val);
  return (1);
}

static int user_from_string (struct option_t* dst, const char* val,
                             char* errbuf, size_t errlen) {
  /* if dst == NULL, we may get here in case the user did unset it,
   * see parse_set() where item is free()'d before coming here; so
   * just silently ignore it */
  if (!dst)
    return (1);
  if (str_len ((char*) dst->data) == 0)
    dst->data = (unsigned long) str_dup (val);
  else {
    char* s = (char*) dst->data;
    str_replace (&s, val);
  }
  if (str_len (dst->init) == 0)
    dst->init = str_dup ((char*) dst->data);
  return (1);
}

static void quad_to_string (char* dst, size_t dstlen,
                            struct option_t* option) {
  char *vals[] = { "no", "yes", "ask-no", "ask-yes" };
  snprintf (dst, dstlen, "%s=%s", option->option,
            vals[quadoption (option->data)]);
}

static int quad_from_string (struct option_t* dst, const char* val,
                             char* errbuf, size_t errlen) {
  int flag = -1;

  if (!dst)
    return (0);
  if (ascii_strncasecmp (val, "yes", 3) == 0)
    flag = M_YES;
  else if (ascii_strncasecmp (val, "no", 2) == 0)
    flag = M_NO;
  else if (ascii_strncasecmp (val, "ask-yes", 7) == 0)
    flag = M_ASKYES;
  else if (ascii_strncasecmp (val, "ask-no", 6) == 0)
    flag = M_ASKNO;

  if (flag < 0)
    return (0);

  set_quadoption (dst->data, flag);
  return (1);
}

static void sort_to_string (char* dst, size_t dstlen,
                            struct option_t* option) {
  const struct mapping_t *map = get_sortmap (option);
  char* p = NULL;

  if (!map) {
    snprintf (dst, sizeof (dst), "%s=unknown", option->option);
    return;
  }

  p = mutt_getnamebyvalue (*((short *) option->data) & SORT_MASK,
                           map);

  snprintf (dst, dstlen, "%s=%s%s%s", option->option,
            (*((short *) option->data) & SORT_REVERSE) ?
            "reverse-" : "",
            (*((short *) option->data) & SORT_LAST) ? "last-" :
            "", NONULL (p));
}

static int sort_from_string (struct option_t* dst, const char* val,
                             char* errbuf, size_t errlen) {
  const struct mapping_t *map = NULL;
  if (!(map = get_sortmap (dst))) {
    if (errbuf)
      snprintf (errbuf, errlen, _("%s: Unknown type."),
                dst->option);
    return (0);
  }
  if (parse_sort (dst, val, map, errbuf, errlen) == -1)
    return (0);
  return (1);
}

static void rx_to_string (char* dst, size_t dstlen,
                          struct option_t* option) {
  rx_t* p = (rx_t*) option->data;
  snprintf (dst, dstlen, "%s=\"%s\"", option->option,
            NONULL (p->pattern));
}

static int rx_from_string (struct option_t* dst, const char* val,
                           char* errbuf, size_t errlen) {
  rx_t* p = NULL;
  regex_t* rx = NULL;
  int flags = 0, e = 0, not = 0;
  char* s = NULL;

  if (!dst)
    return (0);

  if (option (OPTATTACHMSG) && !str_cmp (dst->option, "reply_regexp")) {
    if (errbuf)
      snprintf (errbuf, errlen,
                "Operation not permitted when in attach-message mode.");
    return (0);
  }

  if (!((rx_t*) dst->data))
    *((rx_t**) dst->data) = mem_calloc (1, sizeof (rx_t));

  p = (rx_t*) dst->data;

  /* something to do? */
  if (!val || !*val || (p->pattern && str_cmp (p->pattern, val) == 0))
    return (1);

  if (str_cmp (dst->option, "mask") != 0)
    flags |= mutt_which_case (val);

  s = (char*) val;
  if (str_cmp (dst->option, "mask") == 0 && *s == '!') {
    not = 1;
    s++;
  }

  rx = mem_malloc (sizeof (regex_t));

  if ((e = REGCOMP (rx, s, flags)) != 0) {
    regerror (e, rx, errbuf, errlen);
    regfree (rx);
    mem_free (&rx);
    return (0);
  }

  if (p->rx) {
    regfree (p->rx);
    mem_free (&p->rx);
  }

  str_replace (&p->pattern, val);
  p->rx = rx;
  p->not = not;

  if (str_cmp (dst->option, "reply_regexp") == 0)
    mutt_adjust_all_subjects ();

  return (1);
}

static void magic_to_string (char* dst, size_t dstlen,
                             struct option_t* option) {
  const char* s = NULL;
  switch (option->data) {
    case M_MBOX:    s = "mbox"; break;
    case M_MMDF:    s = "MMDF"; break;
    case M_MH:      s = "MH"; break;
    case M_MAILDIR: s = "Maildir"; break;
    default:        s = "unknown"; break;
  }
  snprintf (dst, dstlen, "%s=%s", option->option, s);
}

static int magic_from_string (struct option_t* dst, const char* val,
                              char* errbuf, size_t errlen) {
  int flag = -1;

  if (!dst || !val || !*val)
    return (0);
  if (ascii_strncasecmp (val, "mbox", 4) == 0)
    flag = M_MBOX;
  else if (ascii_strncasecmp (val, "mmdf", 4) == 0)
    flag = M_MMDF;
  else if (ascii_strncasecmp (val, "mh", 2) == 0)
    flag = M_MH;
  else if (ascii_strncasecmp (val, "maildir", 7) == 0)
    flag = M_MAILDIR;

  if (flag < 0)
    return (0);

  *((short*) dst->data) = flag;
  return (1);

}

static void addr_to_string (char* dst, size_t dstlen,
                            struct option_t* option) {
  char s[HUGE_STRING];
  s[0] = '\0';
  rfc822_write_address (s, sizeof (s), *((ADDRESS**) option->data), 0);
  snprintf (dst, dstlen, "%s=\"%s\"", option->option, NONULL (s));
}

static int addr_from_string (struct option_t* dst, const char* val,
                             char* errbuf, size_t errlen) {
  if (!dst)
    return (0);
  rfc822_free_address ((ADDRESS**) dst->data);
  if (val && *val)
    *((ADDRESS**) dst->data) = rfc822_parse_adrlist (NULL, val);
  return (1);
}

int mutt_option_value (const char* val, char* dst, size_t dstlen) {
  struct option_t* option = NULL;
  char* tmp = NULL, *t = NULL;
  size_t l = 0;

  if (!(option = hash_find (ConfigOptions, val))) {
    debug_print (1, ("var '%s' not found\n", val));
    *dst = '\0';
    return (0);
  }
  tmp = mem_malloc (dstlen+1);
  FuncTable[DTYPE (option->type)].opt_to_string (tmp, dstlen, option);

  /* as we get things of type $var=value and don't want to bloat the
   * above "just" for expansion, we do the stripping here */
  debug_print (1, ("orig == '%s'\n", tmp));
  t = strchr (tmp, '=');
  t++;
  l = str_len (t);
  if (l >= 2) {
    if (t[l-1] == '"' && *t == '"') {
      t[l-1] = '\0';
      t++;
    }
  }
  memcpy (dst, t, l+1);
  mem_free (&tmp);
  debug_print (1, ("stripped == '%s'\n", dst));

  return (1);
}

/* for synonym warning reports: adds synonym to end of list */
static void syn_add (struct option_t* n, struct option_t* o) {
  syn_t* tmp = mem_malloc (sizeof (syn_t));
  tmp->f = str_dup (CurRCFile);
  tmp->l = CurRCLine;
  tmp->n = n;
  tmp->o = o;
  list_push_back (&Synonyms, tmp);
}

/* for synonym warning reports: free single item (for list_del()) */
static void syn_del (void** p) {
  mem_free(&(*(syn_t**) p)->f);
  mem_free(p);
}

void toggle_quadoption (int opt)
{
  int n = opt / 4;
  int b = (opt % 4) * 2;

  QuadOptions[n] ^= (1 << b);
}

void set_quadoption (int opt, int flag)
{
  int n = opt / 4;
  int b = (opt % 4) * 2;

  QuadOptions[n] &= ~(0x3 << b);
  QuadOptions[n] |= (flag & 0x3) << b;
}

int quadoption (int opt)
{
  int n = opt / 4;
  int b = (opt % 4) * 2;

  return (QuadOptions[n] >> b) & 0x3;
}

int query_quadoption (int opt, const char *prompt)
{
  int v = quadoption (opt);

  switch (v) {
  case M_YES:
  case M_NO:
    return (v);

  default:
    v = mutt_yesorno (prompt, (v == M_ASKYES));
    CLEARLINE (LINES - 1);
    return (v);
  }

  /* not reached */
}

static void add_to_list (LIST ** list, const char *str)
{
  LIST *t, *last = NULL;

  /* don't add a NULL or empty string to the list */
  if (!str || *str == '\0')
    return;

  /* check to make sure the item is not already on this list */
  for (last = *list; last; last = last->next) {
    if (ascii_strcasecmp (str, last->data) == 0) {
      /* already on the list, so just ignore it */
      last = NULL;
      break;
    }
    if (!last->next)
      break;
  }

  if (!*list || last) {
    t = (LIST *) mem_calloc (1, sizeof (LIST));
    t->data = str_dup (str);
    if (last) {
      last->next = t;
      last = last->next;
    }
    else
      *list = last = t;
  }
}

static int add_to_rx_list (list2_t** list, const char *s, int flags,
                           BUFFER * err)
{
  rx_t* rx;
  int i = 0;

  if (!s || !*s)
    return 0;

  if (!(rx = rx_compile (s, flags))) {
    snprintf (err->data, err->dsize, "Bad regexp: %s\n", s);
    return -1;
  }

  i = rx_lookup ((*list), rx->pattern);
  if (i >= 0)
    rx_free (&rx);
  else
    list_push_back (list, rx);
  return 0;
}

static int add_to_spam_list (SPAM_LIST ** list, const char *pat,
                             const char *templ, BUFFER * err)
{
  SPAM_LIST *t = NULL, *last = NULL;
  rx_t* rx;
  int n;
  const char *p;

  if (!pat || !*pat || !templ)
    return 0;

  if (!(rx = rx_compile (pat, REG_ICASE))) {
    snprintf (err->data, err->dsize, _("Bad regexp: %s"), pat);
    return -1;
  }

  /* check to make sure the item is not already on this list */
  for (last = *list; last; last = last->next) {
    if (ascii_strcasecmp (rx->pattern, last->rx->pattern) == 0) {
      /* Already on the list. Formerly we just skipped this case, but
       * now we're supporting removals, which means we're supporting
       * re-adds conceptually. So we probably want this to imply a
       * removal, then do an add. We can achieve the removal by freeing
       * the template, and leaving t pointed at the current item.
       */
      t = last;
      mem_free(t->template);
      break;
    }
    if (!last->next)
      break;
  }

  /* If t is set, it's pointing into an extant SPAM_LIST* that we want to
   * update. Otherwise we want to make a new one to link at the list's end.
   */
  if (!t) {
    t = mutt_new_spam_list ();
    t->rx = rx;
    if (last)
      last->next = t;
    else
      *list = t;
  }

  /* Now t is the SPAM_LIST* that we want to modify. It is prepared. */
  t->template = str_dup (templ);

  /* Find highest match number in template string */
  t->nmatch = 0;
  for (p = templ; *p;) {
    if (*p == '%') {
      n = atoi (++p);
      if (n > t->nmatch)
        t->nmatch = n;
      while (*p && isdigit ((int) *p))
        ++p;
    }
    else
      ++p;
  }
  t->nmatch++;                  /* match 0 is always the whole expr */

  return 0;
}

static int remove_from_spam_list (SPAM_LIST ** list, const char *pat)
{
  SPAM_LIST *spam, *prev;
  int nremoved = 0;

  /* Being first is a special case. */
  spam = *list;
  if (!spam)
    return 0;
  if (spam->rx && !str_cmp (spam->rx->pattern, pat)) {
    *list = spam->next;
    rx_free (&spam->rx);
    mem_free(&spam->template);
    mem_free(&spam);
    return 1;
  }

  prev = spam;
  for (spam = prev->next; spam;) {
    if (!str_cmp (spam->rx->pattern, pat)) {
      prev->next = spam->next;
      rx_free (&spam->rx);
      mem_free(spam->template);
      mem_free(spam);
      spam = prev->next;
      ++nremoved;
    }
    else
      spam = spam->next;
  }

  return nremoved;
}


static void remove_from_list (LIST ** l, const char *str)
{
  LIST *p, *last = NULL;

  if (str_cmp ("*", str) == 0)
    mutt_free_list (l);         /* ``unCMD *'' means delete all current entries */
  else {
    p = *l;
    last = NULL;
    while (p) {
      if (ascii_strcasecmp (str, p->data) == 0) {
        mem_free (&p->data);
        if (last)
          last->next = p->next;
        else
          (*l) = p->next;
        mem_free (&p);
      }
      else {
        last = p;
        p = p->next;
      }
    }
  }
}

static int remove_from_rx_list (list2_t** l, const char *str)
{
  int i = 0;

  if (str_cmp ("*", str) == 0) {
    list_del (l, (list_del_t*) rx_free);
    return (0);
  }
  else {
    i = rx_lookup ((*l), str);
    if (i >= 0) {
      rx_t* r = list_pop_idx ((*l), i);
      rx_free (&r);
      return (0);
    }
  }
  return (-1);
}

static int parse_ifdef (BUFFER * tmp, BUFFER * s, unsigned long data,
                        BUFFER * err)
{
  int i, j, res = 0;
  BUFFER token;
  struct option_t* option = NULL;

  memset (&token, 0, sizeof (token));
  mutt_extract_token (tmp, s, 0);

  /* is the item defined as a variable or a function? */
  if ((option = hash_find (ConfigOptions, tmp->data)) != NULL)
    res = 1;
  else {
    for (i = 0; !res && i < MENU_MAX; i++) {
      struct binding_t *b = km_get_table (Menus[i].value);

      if (!b)
        continue;

      for (j = 0; b[j].name; j++)
        if (!ascii_strncasecmp (tmp->data, b[j].name, str_len (tmp->data))
            && (str_len (b[j].name) == str_len (tmp->data))) {
          res = 1;
          break;
        }
    }
  }
  /* check for feature_* */
  if (!res && ascii_strncasecmp (tmp->data, "feature_", 8) == 0 &&
      (j = str_len (tmp->data)) > 8) {
    i = 0;
    while (Features[i]) {
      if (str_len (Features[i]) == j-8 &&
          ascii_strncasecmp (Features[i], tmp->data+8, j-8) == 0) {
        res = 1;
        break;
      }
      i++;
    }
  }

  if (!MoreArgs (s)) {
    if (data)
      snprintf (err->data, err->dsize, _("ifdef: too few arguments"));
    else
      snprintf (err->data, err->dsize, _("ifndef: too few arguments"));
    return (-1);
  }

  mutt_extract_token (tmp, s, M_TOKEN_SPACE);

  if (data == res) {
    if (mutt_parse_rc_line (tmp->data, &token, err) == -1) {
      mutt_error ("Error: %s", err->data);
      mem_free (&token.data);
      return (-1);
    }
    mem_free (&token.data);
  }
  return 0;
}

static int parse_unignore (BUFFER * buf, BUFFER * s, unsigned long data,
                           BUFFER * err)
{
  do {
    mutt_extract_token (buf, s, 0);

    /* don't add "*" to the unignore list */
    if (strcmp (buf->data, "*"))
      add_to_list (&UnIgnore, buf->data);

    remove_from_list (&Ignore, buf->data);
  }
  while (MoreArgs (s));

  return 0;
}

static int parse_ignore (BUFFER * buf, BUFFER * s, unsigned long data,
                         BUFFER * err)
{
  do {
    mutt_extract_token (buf, s, 0);
    remove_from_list (&UnIgnore, buf->data);
    add_to_list (&Ignore, buf->data);
  }
  while (MoreArgs (s));

  return 0;
}

static int parse_list (BUFFER * buf, BUFFER * s, unsigned long data,
                       BUFFER * err)
{
  do {
    mutt_extract_token (buf, s, 0);
    add_to_list ((LIST **) data, buf->data);
  }
  while (MoreArgs (s));

  return 0;
}

static void _alternates_clean (void)
{
  int i;

  if (Context && Context->msgcount) {
    for (i = 0; i < Context->msgcount; i++)
      Context->hdrs[i]->recip_valid = 0;
  }
}

static int parse_alternates (BUFFER * buf, BUFFER * s, unsigned long data,
                             BUFFER * err)
{
  _alternates_clean ();
  do {
    mutt_extract_token (buf, s, 0);
    remove_from_rx_list (&UnAlternates, buf->data);

    if (add_to_rx_list (&Alternates, buf->data, REG_ICASE, err) != 0)
      return -1;
  }
  while (MoreArgs (s));

  return 0;
}

static int parse_unalternates (BUFFER * buf, BUFFER * s, unsigned long data,
                               BUFFER * err)
{
  _alternates_clean ();
  do {
    mutt_extract_token (buf, s, 0);
    remove_from_rx_list (&Alternates, buf->data);

    if (str_cmp (buf->data, "*") &&
        add_to_rx_list (&UnAlternates, buf->data, REG_ICASE, err) != 0)
      return -1;

  }
  while (MoreArgs (s));

  return 0;
}

static int parse_spam_list (BUFFER * buf, BUFFER * s, unsigned long data,
                            BUFFER * err)
{
  BUFFER templ;

  memset (&templ, 0, sizeof (templ));

  /* Insist on at least one parameter */
  if (!MoreArgs (s)) {
    if (data == M_SPAM)
      strfcpy (err->data, _("spam: no matching pattern"), err->dsize);
    else
      strfcpy (err->data, _("nospam: no matching pattern"), err->dsize);
    return -1;
  }

  /* Extract the first token, a regexp */
  mutt_extract_token (buf, s, 0);

  /* data should be either M_SPAM or M_NOSPAM. M_SPAM is for spam commands. */
  if (data == M_SPAM) {
    /* If there's a second parameter, it's a template for the spam tag. */
    if (MoreArgs (s)) {
      mutt_extract_token (&templ, s, 0);

      /* Add to the spam list. */
      if (add_to_spam_list (&SpamList, buf->data, templ.data, err) != 0) {
        mem_free (&templ.data);
        return -1;
      }
      mem_free (&templ.data);
    }

    /* If not, try to remove from the nospam list. */
    else {
      remove_from_rx_list (&NoSpamList, buf->data);
    }

    return 0;
  }

  /* M_NOSPAM is for nospam commands. */
  else if (data == M_NOSPAM) {
    /* nospam only ever has one parameter. */

    /* "*" is a special case. */
    if (!str_cmp (buf->data, "*")) {
      mutt_free_spam_list (&SpamList);
      list_del (&NoSpamList, (list_del_t*) rx_free);
      return 0;
    }

    /* If it's on the spam list, just remove it. */
    if (remove_from_spam_list (&SpamList, buf->data) != 0)
      return 0;

    /* Otherwise, add it to the nospam list. */
    if (add_to_rx_list (&NoSpamList, buf->data, REG_ICASE, err) != 0)
      return -1;

    return 0;
  }

  /* This should not happen. */
  strfcpy (err->data, "This is no good at all.", err->dsize);
  return -1;
}

static int parse_unlist (BUFFER * buf, BUFFER * s, unsigned long data,
                         BUFFER * err)
{
  do {
    mutt_extract_token (buf, s, 0);
    /*
     * Check for deletion of entire list
     */
    if (str_cmp (buf->data, "*") == 0) {
      mutt_free_list ((LIST **) data);
      break;
    }
    remove_from_list ((LIST **) data, buf->data);
  }
  while (MoreArgs (s));

  return 0;
}

static int parse_lists (BUFFER * buf, BUFFER * s, unsigned long data,
                        BUFFER * err)
{
  do {
    mutt_extract_token (buf, s, 0);
    remove_from_rx_list (&UnMailLists, buf->data);

    if (add_to_rx_list (&MailLists, buf->data, REG_ICASE, err) != 0)
      return -1;
  }
  while (MoreArgs (s));

  return 0;
}

/* always wise to do what someone else did before */
static void _attachments_clean (void) {
  int i;
  if (Context && Context->msgcount) {
    for (i = 0; i < Context->msgcount; i++)
      Context->hdrs[i]->attach_valid = 0;
  }
}

static int parse_attach_list (BUFFER *buf, BUFFER *s, LIST **ldata,
                              BUFFER *err) {
  ATTACH_MATCH *a;
  LIST *listp, *lastp;
  char *p;
  char *tmpminor;
  int len;

  /* Find the last item in the list that data points to. */
  lastp = NULL;
  debug_print (5, ("parse_attach_list: ldata = %08x, *ldata = %08x\n",
                   (unsigned int)ldata, (unsigned int)*ldata));
  for (listp = *ldata; listp; listp = listp->next) {
    a = (ATTACH_MATCH *)listp->data;
    debug_print (5, ("parse_attach_list: skipping %s/%s\n", a->major, a->minor));
    lastp = listp;
  }

  do {
    mutt_extract_token (buf, s, 0);

    if (!buf->data || *buf->data == '\0')
      continue;
    
    a = mem_malloc(sizeof(ATTACH_MATCH));
    
    /* some cheap hacks that I expect to remove */
    if (!str_casecmp(buf->data, "any"))
      a->major = str_dup("*/.*");
    else if (!str_casecmp(buf->data, "none"))
      a->major = str_dup("cheap_hack/this_should_never_match");
    else
      a->major = str_dup(buf->data);
    
    if ((p = strchr(a->major, '/'))) {
      *p = '\0';
      ++p;
      a->minor = p;
    } else {
      a->minor = "unknown";
    }

    len = str_len (a->minor);
    tmpminor = mem_malloc(len+3);
    strcpy(&tmpminor[1], a->minor); /* __STRCPY_CHECKED__ */
    tmpminor[0] = '^';
    tmpminor[len+1] = '$';
    tmpminor[len+2] = '\0';
    
    a->major_int = mutt_check_mime_type(a->major);
    regcomp(&a->minor_rx, tmpminor, REG_ICASE|REG_EXTENDED);
    
    mem_free (&tmpminor);

    debug_print (5, ("parse_attach_list: added %s/%s [%d]\n",
                     a->major, a->minor, a->major_int));

    listp = mem_malloc(sizeof(LIST));
    listp->data = (char *)a;
    listp->next = NULL;
    if (lastp) {
      lastp->next = listp;
    } else {
      *ldata = listp;
    }
    lastp = listp;
  }
  while (MoreArgs (s));

  _attachments_clean();
  return 0;
}

static int parse_unattach_list (BUFFER *buf, BUFFER *s, LIST **ldata, BUFFER *err) {
  ATTACH_MATCH *a;
  LIST *lp, *lastp, *newlp;
  char *tmp;
  int major;
  char *minor;

  do {
    mutt_extract_token (buf, s, 0);
    
    if (!str_casecmp(buf->data, "any"))
      tmp = str_dup("*/.*");
    else if (!str_casecmp(buf->data, "none"))
      tmp = str_dup("cheap_hack/this_should_never_match");
    else
      tmp = str_dup(buf->data);
    
    if ((minor = strchr(tmp, '/'))) {
      *minor = '\0';
      ++minor;
    } else {
      minor = "unknown";
    }
    major = mutt_check_mime_type(tmp);
    
    /* We must do our own walk here because remove_from_list() will only
     * remove the LIST->data, not anything pointed to by the LIST->data. */
    lastp = NULL;
    for(lp = *ldata; lp; ) {
      a = (ATTACH_MATCH *)lp->data;
      debug_print(5, ("parse_unattach_list: check %s/%s [%d] : %s/%s [%d]\n",
                      a->major, a->minor, a->major_int, tmp, minor, major));
      if (a->major_int == major && !str_casecmp(minor, a->minor)) {
        debug_print(5, ("parse_unattach_list: removed %s/%s [%d]\n",
                        a->major, a->minor, a->major_int));
        regfree(&a->minor_rx);
        mem_free(&a->major);
        
        /* Relink backward */
        if (lastp)
          lastp->next = lp->next;
        else
          *ldata = lp->next;
        
        newlp = lp->next;
        mem_free(&lp->data); /* same as a */
        mem_free(&lp);
        lp = newlp;
        continue;
      }
        
      lastp = lp;
      lp = lp->next;
    }
  }
  while (MoreArgs (s));
  
  mem_free (&tmp);
  _attachments_clean();
  return 0;
}

static int print_attach_list (LIST *lp, char op, char *name) {
  while (lp) {
    printf("attachments %c%s %s/%s\n", op, name,
           ((ATTACH_MATCH *)lp->data)->major,
           ((ATTACH_MATCH *)lp->data)->minor);
    lp = lp->next;
  }
  
  return 0;
}

static int parse_attachments (BUFFER *buf, BUFFER *s, unsigned long data, BUFFER *err) {
  char op, *category;
  LIST **listp;
  
  mutt_extract_token(buf, s, 0);
  if (!buf->data || *buf->data == '\0') {
    strfcpy(err->data, _("attachments: no disposition"), err->dsize);
    return -1;
  }
 
  category = buf->data;
  op = *category++;
  
  if (op == '?') {
    mutt_endwin (NULL);
    fflush (stdout);
    printf("\nCurrent attachments settings:\n\n");
    print_attach_list(AttachAllow, '+', "A");
    print_attach_list(AttachExclude, '-', "A");
    print_attach_list(InlineAllow, '+', "I");
    print_attach_list(InlineExclude, '-', "I");
    set_option (OPTFORCEREDRAWINDEX);
    set_option (OPTFORCEREDRAWPAGER);
    mutt_any_key_to_continue (NULL);
    return 0;
  }

  if (op != '+' && op != '-') {
    op = '+';
    category--;
  }
  if (!str_ncasecmp(category, "attachment", strlen(category))) {
    if (op == '+')
      listp = &AttachAllow;
    else
      listp = &AttachExclude;
  }
  else if (!str_ncasecmp(category, "inline", strlen(category))) {
    if (op == '+')
      listp = &InlineAllow;
    else
      listp = &InlineExclude;
  } else {
    strfcpy(err->data, _("attachments: invalid disposition"), err->dsize);
    return -1;
  }
  
  return parse_attach_list(buf, s, listp, err);
}

static int parse_unattachments (BUFFER *buf, BUFFER *s, unsigned long data, BUFFER *err) {
  char op, *p;
  LIST **listp;
  
  mutt_extract_token(buf, s, 0);
  if (!buf->data || *buf->data == '\0') {
    strfcpy(err->data, _("unattachments: no disposition"), err->dsize);
    return -1;
  }
  
  p = buf->data;
  op = *p++;
  if (op != '+' && op != '-') {
    op = '+';
    p--;
  }
  if (!str_ncasecmp(p, "attachment", strlen(p))) {
    if (op == '+')
      listp = &AttachAllow;
    else
      listp = &AttachExclude;
  }
  else if (!str_ncasecmp(p, "inline", strlen(p))) {
    if (op == '+')
      listp = &InlineAllow;
    else
      listp = &InlineExclude;
  }
  else {
    strfcpy(err->data, _("unattachments: invalid disposition"), err->dsize);
    return -1;
  }
  
  return parse_unattach_list(buf, s, listp, err);
}

static int parse_unlists (BUFFER * buf, BUFFER * s, unsigned long data,
                          BUFFER * err)
{
  do {
    mutt_extract_token (buf, s, 0);
    remove_from_rx_list (&SubscribedLists, buf->data);
    remove_from_rx_list (&MailLists, buf->data);

    if (str_cmp (buf->data, "*") &&
        add_to_rx_list (&UnMailLists, buf->data, REG_ICASE, err) != 0)
      return -1;
  }
  while (MoreArgs (s));

  return 0;
}

static int parse_subscribe (BUFFER * buf, BUFFER * s, unsigned long data,
                            BUFFER * err)
{
  do {
    mutt_extract_token (buf, s, 0);
    remove_from_rx_list (&UnMailLists, buf->data);
    remove_from_rx_list (&UnSubscribedLists, buf->data);

    if (add_to_rx_list (&MailLists, buf->data, REG_ICASE, err) != 0)
      return -1;
    if (add_to_rx_list (&SubscribedLists, buf->data, REG_ICASE, err) != 0)
      return -1;
  }
  while (MoreArgs (s));

  return 0;
}

static int parse_unsubscribe (BUFFER * buf, BUFFER * s, unsigned long data,
                              BUFFER * err)
{
  do {
    mutt_extract_token (buf, s, 0);
    remove_from_rx_list (&SubscribedLists, buf->data);

    if (str_cmp (buf->data, "*") &&
        add_to_rx_list (&UnSubscribedLists, buf->data, REG_ICASE, err) != 0)
      return -1;
  }
  while (MoreArgs (s));

  return 0;
}

static int parse_unalias (BUFFER * buf, BUFFER * s, unsigned long data,
                          BUFFER * err)
{
  ALIAS *tmp, *last = NULL;

  do {
    mutt_extract_token (buf, s, 0);

    if (str_cmp ("*", buf->data) == 0) {
      if (CurrentMenu == MENU_ALIAS) {
        for (tmp = Aliases; tmp; tmp = tmp->next)
          tmp->del = 1;
        set_option (OPTFORCEREDRAWINDEX);
      }
      else
        mutt_free_alias (&Aliases);
      break;
    }
    else
      for (tmp = Aliases; tmp; tmp = tmp->next) {
        if (str_casecmp (buf->data, tmp->name) == 0) {
          if (CurrentMenu == MENU_ALIAS) {
            tmp->del = 1;
            set_option (OPTFORCEREDRAWINDEX);
            break;
          }

          if (last)
            last->next = tmp->next;
          else
            Aliases = tmp->next;
          tmp->next = NULL;
          mutt_free_alias (&tmp);
          break;
        }
        last = tmp;
      }
  }
  while (MoreArgs (s));
  return 0;
}

static int parse_alias (BUFFER * buf, BUFFER * s, unsigned long data,
                        BUFFER * err)
{
  ALIAS *tmp = Aliases;
  ALIAS *last = NULL;
  char *estr = NULL;

  if (!MoreArgs (s)) {
    strfcpy (err->data, _("alias: no address"), err->dsize);
    return (-1);
  }

  mutt_extract_token (buf, s, 0);

  debug_print (2, ("first token is '%s'.\n", buf->data));

  /* check to see if an alias with this name already exists */
  for (; tmp; tmp = tmp->next) {
    if (!str_casecmp (tmp->name, buf->data))
      break;
    last = tmp;
  }

  if (!tmp) {
    /* create a new alias */
    tmp = (ALIAS *) mem_calloc (1, sizeof (ALIAS));
    tmp->self = tmp;
    tmp->name = str_dup (buf->data);
    /* give the main addressbook code a chance */
    if (CurrentMenu == MENU_ALIAS)
      set_option (OPTMENUCALLER);
  }
  else {
    /* override the previous value */
    rfc822_free_address (&tmp->addr);
    if (CurrentMenu == MENU_ALIAS)
      set_option (OPTFORCEREDRAWINDEX);
  }

  mutt_extract_token (buf, s,
                      M_TOKEN_QUOTE | M_TOKEN_SPACE | M_TOKEN_SEMICOLON);
  debug_print (2, ("second token is '%s'.\n", buf->data));
  tmp->addr = mutt_parse_adrlist (tmp->addr, buf->data);
  if (last)
    last->next = tmp;
  else
    Aliases = tmp;
  if (mutt_addrlist_to_idna (tmp->addr, &estr)) {
    snprintf (err->data, err->dsize,
              _("Warning: Bad IDN '%s' in alias '%s'.\n"), estr, tmp->name);
    return -1;
  }
#ifdef DEBUG
  if (DebugLevel >= 2) {
    ADDRESS *a;

    /* A group is terminated with an empty address, so check a->mailbox */
    for (a = tmp->addr; a && a->mailbox; a = a->next) {
      if (!a->group)
        debug_print (2, ("%s\n", a->mailbox));
      else
        debug_print (2, ("group %s\n", a->mailbox));
    }
  }
#endif
  return 0;
}

static int
parse_unmy_hdr (BUFFER * buf, BUFFER * s, unsigned long data, BUFFER * err)
{
  LIST *last = NULL;
  LIST *tmp = UserHeader;
  LIST *ptr;
  size_t l;

  do {
    mutt_extract_token (buf, s, 0);
    if (str_cmp ("*", buf->data) == 0)
      mutt_free_list (&UserHeader);
    else {
      tmp = UserHeader;
      last = NULL;

      l = str_len (buf->data);
      if (buf->data[l - 1] == ':')
        l--;

      while (tmp) {
        if (ascii_strncasecmp (buf->data, tmp->data, l) == 0
            && tmp->data[l] == ':') {
          ptr = tmp;
          if (last)
            last->next = tmp->next;
          else
            UserHeader = tmp->next;
          tmp = tmp->next;
          ptr->next = NULL;
          mutt_free_list (&ptr);
        }
        else {
          last = tmp;
          tmp = tmp->next;
        }
      }
    }
  }
  while (MoreArgs (s));
  return 0;
}

static int parse_my_hdr (BUFFER * buf, BUFFER * s, unsigned long data,
                         BUFFER * err)
{
  LIST *tmp;
  size_t keylen;
  char *p;

  mutt_extract_token (buf, s, M_TOKEN_SPACE | M_TOKEN_QUOTE);
  if ((p = strpbrk (buf->data, ": \t")) == NULL || *p != ':') {
    strfcpy (err->data, _("invalid header field"), err->dsize);
    return (-1);
  }
  keylen = p - buf->data + 1;

  if (UserHeader) {
    for (tmp = UserHeader;; tmp = tmp->next) {
      /* see if there is already a field by this name */
      if (ascii_strncasecmp (buf->data, tmp->data, keylen) == 0) {
        /* replace the old value */
        mem_free (&tmp->data);
        tmp->data = buf->data;
        memset (buf, 0, sizeof (BUFFER));
        return 0;
      }
      if (!tmp->next)
        break;
    }
    tmp->next = mutt_new_list ();
    tmp = tmp->next;
  }
  else {
    tmp = mutt_new_list ();
    UserHeader = tmp;
  }
  tmp->data = buf->data;
  memset (buf, 0, sizeof (BUFFER));
  return 0;
}

static int
parse_sort (struct option_t* dst, const char *s, const struct mapping_t *map,
            char* errbuf, size_t errlen) {
  int i, flags = 0;

  if (str_ncmp ("reverse-", s, 8) == 0) {
    s += 8;
    flags = SORT_REVERSE;
  }

  if (str_ncmp ("last-", s, 5) == 0) {
    s += 5;
    flags |= SORT_LAST;
  }

  if ((i = mutt_getvaluebyname (s, map)) == -1) {
    if (errbuf)
      snprintf (errbuf, errlen, _("'%s' is invalid for $%s"), s, dst->option);
    return (-1);
  }

  *((short*) dst->data) = i | flags;
  return 0;
}

/* if additional data more == 1, we want to resolve synonyms */
static void mutt_set_default (const char* name, void* p, unsigned long more) {
  char buf[LONG_STRING];
  struct option_t* ptr = (struct option_t*) p;

  if (DTYPE (ptr->type) == DT_SYN) {
    if (!more)
      return;
    ptr = hash_find (ConfigOptions, (char*) ptr->data);
  }
  if (!ptr || *ptr->init || !FuncTable[DTYPE (ptr->type)].opt_from_string)
    return;
  mutt_option_value (ptr->option, buf, sizeof (buf));
  if (str_len (ptr->init) == 0 && buf && *buf)
    ptr->init = str_dup (buf);
}

static struct option_t* add_option (const char* name, const char* init,
                                    short type, short dup) {
  struct option_t* option = mem_calloc (1, sizeof (struct option_t));

  debug_print (1, ("adding $%s\n", name));

  option->option = str_dup (name);
  option->type = type;
  if (init)
    option->init = dup ? str_dup (init) : (char*) init;
  return (option);
}

/* creates new option_t* of type DT_USER for $user_ var */
static struct option_t* add_user_option (const char* name) {
  return (add_option (name, NULL, DT_USER, 1));
}

/* free()'s option_t* */
static void del_option (void* p) {
  struct option_t* ptr = (struct option_t*) p;
  char* s = (char*) ptr->data;
  debug_print (1, ("removing option '%s' from table\n", NONULL (ptr->option)));
  mem_free (&ptr->option);
  mem_free (&s);
  mem_free (&ptr->init);
  mem_free (&ptr);
}

static int init_expand (char** dst, struct option_t* src) {
  BUFFER token, in;
  size_t len = 0;

  mem_free (dst);

  if (DTYPE(src->type) == DT_STR ||
      DTYPE(src->type) == DT_PATH) {
    /* only expand for string as it's the only place where
     * we want to expand vars right now */
    if (src->init && *src->init) {
      memset (&token, 0, sizeof (BUFFER));
      memset (&in, 0, sizeof (BUFFER));
      len = str_len (src->init) + 2;
      in.data = mem_malloc (len+1);
      snprintf (in.data, len, "\"%s\"", src->init);
      in.dptr = in.data;
      in.dsize = len;
      mutt_extract_token (&token, &in, 0);
      if (token.data && *token.data)
        *dst = str_dup (token.data);
      else
        *dst = str_dup ("");
      mem_free (&in.data);
      mem_free (&token.data);
    } else
      *dst = str_dup ("");
  } else
    /* for non-string: take value as is */
    *dst = str_dup (src->init);
  return (1);
}

/* if additional data more == 1, we want to resolve synonyms */
static void mutt_restore_default (const char* name, void* p,
                                  unsigned long more) {
  char errbuf[STRING];
  struct option_t* ptr = (struct option_t*) p;
  char* init = NULL;

  if (DTYPE (ptr->type) == DT_SYN) {
    if (!more)
      return;
    ptr = hash_find (ConfigOptions, (char*) ptr->data);
  }
  if (!ptr)
    return;
  if (FuncTable[DTYPE (ptr->type)].opt_from_string) {
    init_expand (&init, ptr);
    if (!FuncTable[DTYPE (ptr->type)].opt_from_string (ptr, init, errbuf,
                                                       sizeof (errbuf))) {
      if (!option (OPTNOCURSES))
        mutt_endwin (NULL);
      fprintf (stderr, _("Invalid default setting for $%s found: \"%s\".\n"
                         "Please report this error: \"%s\"\n"),
               ptr->option, NONULL (init), errbuf);
      exit (1);
    }
    mem_free (&init);
  }

  if (ptr->flags & R_INDEX)
    set_option (OPTFORCEREDRAWINDEX);
  if (ptr->flags & R_PAGER)
    set_option (OPTFORCEREDRAWPAGER);
  if (ptr->flags & R_RESORT_SUB)
    set_option (OPTSORTSUBTHREADS);
  if (ptr->flags & R_RESORT)
    set_option (OPTNEEDRESORT);
  if (ptr->flags & R_RESORT_INIT)
    set_option (OPTRESORTINIT);
  if (ptr->flags & R_TREE)
    set_option (OPTREDRAWTREE);
}

/* check whether value for $dsn_return would be valid */
static int check_dsn_return (const char* option, unsigned long p,
                             char* errbuf, size_t errlen) {
  char* val = (char*) p;
  if (val && *val && str_ncmp (val, "hdrs", 4) != 0 &&
      str_ncmp (val, "full", 4) != 0) {
    if (errbuf)
      snprintf (errbuf, errlen, _("'%s' is invalid for $%s"), val, "dsn_return");
    return (0);
  }
  return (1);
}

/* check whether value for $dsn_notify would be valid */
static int check_dsn_notify (const char* option, unsigned long p,
                             char* errbuf, size_t errlen) {
  list2_t* list = NULL;
  int i = 0, rc = 1;
  char* val = (char*) p;

  if (!val || !*val)
    return (1);
  list = list_from_str (val, ",");
  if (list_empty (list))
    return (1);

  for (i = 0; i < list->length; i++)
    if (str_ncmp (list->data[i], "never", 5) != 0 &&
        str_ncmp (list->data[i], "failure", 7) != 0 &&
        str_ncmp (list->data[i], "delay", 5) != 0 &&
        str_ncmp (list->data[i], "success", 7) != 0) {
      if (errbuf)
        snprintf (errbuf, errlen, _("'%s' is invalid for $%s"),
                  (char*) list->data[i], "dsn_notify");
      rc = 0;
      break;
    }
  list_del (&list, (list_del_t*) _mem_free);
  return (rc);
}

static int check_num (const char* option, unsigned long p,
                      char* errbuf, size_t errlen) {
  if ((int) p < 0) {
    if (errbuf)
      snprintf (errbuf, errlen, _("'%d' is invalid for $%s"), (int) p, option);
    return (0);
  }
  return (1);
}

#ifdef DEBUG
static int check_debug (const char* option, unsigned long p,
                        char* errbuf, size_t errlen) {
  if ((int) p <= DEBUG_MAX_LEVEL &&
      (int) p >= DEBUG_MIN_LEVEL)
    return (1);

  if (errbuf)
    snprintf (errbuf, errlen, _("'%d' is invalid for $%s"), (int) p, option);
  return (0);
}
#endif

static int check_history (const char* option, unsigned long p,
                          char* errbuf, size_t errlen) {
  if (!check_num ("history", p, errbuf, errlen))
    return (0);
  mutt_init_history ();
  return (1);
}

static int check_special (const char* name, unsigned long val,
                          char* errbuf, size_t errlen) {
  int i = 0;

  for (i = 0; SpecialVars[i].name; i++) {
    if (str_cmp (SpecialVars[i].name, name) == 0) {
      return (SpecialVars[i].check (SpecialVars[i].name,
                                    val, errbuf, errlen));
    }
  }
  return (1);
}

static const struct mapping_t* get_sortmap (struct option_t* option) {
  const struct mapping_t* map = NULL;

  switch (option->type & DT_SUBTYPE_MASK) {
  case DT_SORT_ALIAS:
    map = SortAliasMethods;
    break;
  case DT_SORT_BROWSER:
    map = SortBrowserMethods;
    break;
  case DT_SORT_KEYS:
    if ((WithCrypto & APPLICATION_PGP))
      map = SortKeyMethods;
    break;
  case DT_SORT_AUX:
    map = SortAuxMethods;
    break;
  default:
    map = SortMethods;
    break;
  }
  return (map);
}

static int parse_set (BUFFER * tmp, BUFFER * s, unsigned long data,
                      BUFFER * err)
{
  int query, unset, inv, reset, r = 0;
  struct option_t* option = NULL;

  while (MoreArgs (s)) {
    /* reset state variables */
    query = 0;
    unset = data & M_SET_UNSET;
    inv = data & M_SET_INV;
    reset = data & M_SET_RESET;

    if (*s->dptr == '?') {
      query = 1;
      s->dptr++;
    }
    else if (str_ncmp ("no", s->dptr, 2) == 0) {
      s->dptr += 2;
      unset = !unset;
    }
    else if (str_ncmp ("inv", s->dptr, 3) == 0) {
      s->dptr += 3;
      inv = !inv;
    }
    else if (*s->dptr == '&') {
      reset = 1;
      s->dptr++;
    }

    /* get the variable name */
    mutt_extract_token (tmp, s, M_TOKEN_EQUAL);

    /* resolve synonyms */
    if ((option = hash_find (ConfigOptions, tmp->data)) != NULL && 
        DTYPE (option->type == DT_SYN)) {
      struct option_t* newopt = hash_find (ConfigOptions, (char*) option->data);
      syn_add (newopt, option);
      option = newopt;
    }

    /* see if we need to add $user_ var */
    if (!option && ascii_strncmp ("user_", tmp->data, 5) == 0) {
      /* there's no option named like this yet so only add one
       * if the action isn't any of: reset, unset, query */
      if (!(reset || unset || query || *s->dptr != '=')) {
        debug_print (1, ("adding user option '%s'\n", tmp->data));
        option = add_user_option (tmp->data);
        hash_insert (ConfigOptions, option->option, option, 0);
      }
    }

    if (!option && !(reset && str_cmp ("all", tmp->data) == 0)) {
      snprintf (err->data, err->dsize, _("%s: unknown variable"), tmp->data);
      return (-1);
    }
    SKIPWS (s->dptr);

    if (reset) {
      if (query || unset || inv) {
        snprintf (err->data, err->dsize, _("prefix is illegal with reset"));
        return (-1);
      }

      if (s && *s->dptr == '=') {
        snprintf (err->data, err->dsize, _("value is illegal with reset"));
        return (-1);
      }

      if (!str_cmp ("all", tmp->data)) {
        if (CurrentMenu == MENU_PAGER) {
          snprintf (err->data, err->dsize, _("Not available in this menu."));
          return (-1);
        }
        hash_map (ConfigOptions, mutt_restore_default, 1);
        set_option (OPTFORCEREDRAWINDEX);
        set_option (OPTFORCEREDRAWPAGER);
        set_option (OPTSORTSUBTHREADS);
        set_option (OPTNEEDRESORT);
        set_option (OPTRESORTINIT);
        set_option (OPTREDRAWTREE);
        return (0);
      }
      else if (!FuncTable[DTYPE (option->type)].opt_from_string) {
        snprintf (err->data, err->dsize, _("$%s is read-only"), option->option);
        r = -1;
        break;
      } else {
        CHECK_PAGER;
        mutt_restore_default (NULL, option, 1);
      }
    }
    else if (DTYPE (option->type) == DT_BOOL) {
      /* XXX this currently ignores the function table
       * as we don't get invert and stuff into it */
      if (s && *s->dptr == '=') {
        if (unset || inv || query) {
          snprintf (err->data, err->dsize, "Usage: set variable=yes|no");
          return (-1);
        }

        s->dptr++;
        mutt_extract_token (tmp, s, 0);
        if (ascii_strcasecmp ("yes", tmp->data) == 0)
          unset = inv = 0;
        else if (ascii_strcasecmp ("no", tmp->data) == 0)
          unset = 1;
        else {
          snprintf (err->data, err->dsize, "Usage: set variable=yes|no");
          return (-1);
        }
      }

      if (query) {
        bool_to_string (err->data, err->dsize, option);
        return 0;
      }

      CHECK_PAGER;
      if (unset)
        unset_option (option->data);
      else if (inv)
        toggle_option (option->data);
      else
        set_option (option->data);
    }
    else if (DTYPE (option->type) == DT_STR ||
             DTYPE (option->type) == DT_PATH ||
             DTYPE (option->type) == DT_ADDR ||
             DTYPE (option->type) == DT_MAGIC ||
             DTYPE (option->type) == DT_NUM ||
             DTYPE (option->type) == DT_SORT ||
             DTYPE (option->type) == DT_RX ||
             DTYPE (option->type) == DT_USER ||
             DTYPE (option->type) == DT_SYS) {

      /* XXX maybe we need to get unset into handlers? */
      if (DTYPE (option->type) == DT_STR ||
          DTYPE (option->type) == DT_PATH ||
          DTYPE (option->type) == DT_ADDR ||
          DTYPE (option->type) == DT_USER ||
          DTYPE (option->type) == DT_SYS) {
        if (unset) {
          CHECK_PAGER;
          if (!FuncTable[DTYPE (option->type)].opt_from_string) {
            snprintf (err->data, err->dsize, _("$%s is read-only"),
                      option->option);
            r = -1;
            break;
          } else if (DTYPE (option->type) == DT_ADDR)
            rfc822_free_address ((ADDRESS **) option->data);
          else if (DTYPE (option->type) == DT_USER)
            /* to unset $user_ means remove */
            hash_delete (ConfigOptions, option->option,
                         option, del_option);
          else
            mem_free ((void *) option->data);
          break;
        }
      }

      if (query || *s->dptr != '=') {
        FuncTable[DTYPE (option->type)].opt_to_string
          (err->data, err->dsize, option);
        break;
      }

      /* the $muttng_ variables are read-only */
      if (!FuncTable[DTYPE (option->type)].opt_from_string) {
        snprintf (err->data, err->dsize, _("$%s is read-only"),
                  option->option);
        r = -1;
        break;
      } else {
        CHECK_PAGER;
        s->dptr++;
        mutt_extract_token (tmp, s, 0);
        if (!FuncTable[DTYPE (option->type)].opt_from_string
            (option, tmp->data, err->data, err->dsize))
          r = -1;
      }
    }
    else if (DTYPE (option->type) == DT_QUAD) {

      if (query) {
        quad_to_string (err->data, err->dsize, option);
        break;
      }

      if (*s->dptr == '=') {
        CHECK_PAGER;
        s->dptr++;
        mutt_extract_token (tmp, s, 0);
        if (ascii_strcasecmp ("yes", tmp->data) == 0)
          set_quadoption (option->data, M_YES);
        else if (ascii_strcasecmp ("no", tmp->data) == 0)
          set_quadoption (option->data, M_NO);
        else if (ascii_strcasecmp ("ask-yes", tmp->data) == 0)
          set_quadoption (option->data, M_ASKYES);
        else if (ascii_strcasecmp ("ask-no", tmp->data) == 0)
          set_quadoption (option->data, M_ASKNO);
        else {
          snprintf (err->data, err->dsize, _("'%s' is invalid for $%s\n"),
                    tmp->data, option->option);
          r = -1;
          break;
        }
      }
      else {
        if (inv)
          toggle_quadoption (option->data);
        else if (unset)
          set_quadoption (option->data, M_NO);
        else
          set_quadoption (option->data, M_YES);
      }
    }
    else {
      snprintf (err->data, err->dsize, _("%s: unknown type"),
                option->option);
      r = -1;
      break;
    }

    if (option->flags & R_INDEX)
      set_option (OPTFORCEREDRAWINDEX);
    if (option->flags & R_PAGER)
      set_option (OPTFORCEREDRAWPAGER);
    if (option->flags & R_RESORT_SUB)
      set_option (OPTSORTSUBTHREADS);
    if (option->flags & R_RESORT)
      set_option (OPTNEEDRESORT);
    if (option->flags & R_RESORT_INIT)
      set_option (OPTRESORTINIT);
    if (option->flags & R_TREE)
      set_option (OPTREDRAWTREE);
  }
  return (r);
}

#define MAXERRS 128

/* reads the specified initialization file.  returns -1 if errors were found
   so that we can pause to let the user know...  */
static int source_rc (const char *rcfile, BUFFER * err)
{
  FILE *f;
  int line = 0, rc = 0, conv = 0;
  BUFFER token;
  char *linebuf = NULL;
  char *currentline = NULL;
  size_t buflen;
  pid_t pid;

  debug_print (2, ("reading configuration file '%s'.\n", rcfile));

  if ((f = mutt_open_read (rcfile, &pid)) == NULL) {
    snprintf (err->data, err->dsize, "%s: %s", rcfile, strerror (errno));
    return (-1);
  }

  memset (&token, 0, sizeof (token));
  while ((linebuf = mutt_read_line (linebuf, &buflen, f, &line)) != NULL) {
    conv = ConfigCharset && (*ConfigCharset) && Charset;
    if (conv) {
      currentline = str_dup (linebuf);
      if (!currentline)
        continue;
      mutt_convert_string (&currentline, ConfigCharset, Charset, 0);
    }
    else
      currentline = linebuf;

    CurRCLine = line;
    CurRCFile = rcfile;

    if (mutt_parse_rc_line (currentline, &token, err) == -1) {
      mutt_error (_("Error in %s, line %d: %s"), rcfile, line, err->data);
      if (--rc < -MAXERRS) {
        if (conv)
          mem_free (&currentline);
        break;
      }
    }
    else {
      if (rc < 0)
        rc = -1;
    }
    if (conv)
      mem_free (&currentline);
  }
  mem_free (&token.data);
  mem_free (&linebuf);
  fclose (f);
  if (pid != -1)
    mutt_wait_filter (pid);
  if (rc) {
    /* the muttrc source keyword */
    snprintf (err->data, err->dsize,
              rc >= -MAXERRS ? _("source: errors in %s")
              : _("source: reading aborted due too many errors in %s"),
              rcfile);
    rc = -1;
  }
  return (rc);
}

#undef MAXERRS

static int parse_source (BUFFER * tmp, BUFFER * s, unsigned long data,
                         BUFFER * err)
{
  char path[_POSIX_PATH_MAX];
  int rc = 0;

  do {
    if (mutt_extract_token (tmp, s, 0) != 0) {
      snprintf (err->data, err->dsize, _("source: error at %s"), s->dptr);
      return (-1);
    }

    strfcpy (path, tmp->data, sizeof (path));
    mutt_expand_path (path, sizeof (path));

    rc += source_rc (path, err);
  }
  while (MoreArgs (s));

  return ((rc < 0) ? -1 : 0);
}

/* line		command to execute

   token	scratch buffer to be used by parser.  caller should free
   		token->data when finished.  the reason for this variable is
		to avoid having to allocate and deallocate a lot of memory
		if we are parsing many lines.  the caller can pass in the
		memory to use, which avoids having to create new space for
		every call to this function.

   err		where to write error messages */
int mutt_parse_rc_line ( /* const */ char *line, BUFFER * token, BUFFER * err)
{
  int i, r = -1;
  BUFFER expn;

  memset (&expn, 0, sizeof (expn));
  expn.data = expn.dptr = line;
  expn.dsize = str_len (line);

  *err->data = 0;

  debug_print (1, ("expand '%s'\n", line));

  SKIPWS (expn.dptr);
  while (*expn.dptr) {
    if (*expn.dptr == '#')
      break;                    /* rest of line is a comment */
    if (*expn.dptr == ';') {
      expn.dptr++;
      continue;
    }
    mutt_extract_token (token, &expn, 0);
    for (i = 0; Commands[i].name; i++) {
      if (!str_cmp (token->data, Commands[i].name)) {
        if (Commands[i].func (token, &expn, Commands[i].data, err) != 0)
          goto finish;
        break;
      }
    }
    if (!Commands[i].name) {
      snprintf (err->data, err->dsize, _("%s: unknown command"),
                NONULL (token->data));
      goto finish;
    }
  }
  r = 0;
finish:
  if (expn.destroy)
    mem_free (&expn.data);
  return (r);
}


#define NUMVARS (sizeof (MuttVars)/sizeof (MuttVars[0]))
#define NUMCOMMANDS (sizeof (Commands)/sizeof (Commands[0]))
/* initial string that starts completion. No telling how much crap 
 * the user has typed so far. Allocate LONG_STRING just to be sure! */
char User_typed[LONG_STRING] = { 0 };

int Num_matched = 0;            /* Number of matches for completion */
char Completed[STRING] = { 0 }; /* completed string (command or variable) */
char *Matches[MAX (NUMVARS, NUMCOMMANDS) + 1];  /* all the matches + User_typed */

/* helper function for completion.  Changes the dest buffer if
   necessary/possible to aid completion.
	dest == completion result gets here.
	src == candidate for completion.
	try == user entered data for completion.
	len == length of dest buffer.
*/
static void candidate (char *dest, char *try, char *src, int len)
{
  int l;

  if (strstr (src, try) == src) {
    Matches[Num_matched++] = src;
    if (dest[0] == 0)
      strfcpy (dest, src, len);
    else {
      for (l = 0; src[l] && src[l] == dest[l]; l++);
      dest[l] = 0;
    }
  }
}

int mutt_command_complete (char *buffer, size_t len, int pos, int numtabs)
{
  char *pt = buffer;
  int num;
  int spaces;                   /* keep track of the number of leading spaces on the line */

  SKIPWS (buffer);
  spaces = buffer - pt;

  pt = buffer + pos - spaces;
  while ((pt > buffer) && !isspace ((unsigned char) *pt))
    pt--;

  if (pt == buffer) {           /* complete cmd */
    /* first TAB. Collect all the matches */
    if (numtabs == 1) {
      Num_matched = 0;
      strfcpy (User_typed, pt, sizeof (User_typed));
      memset (Matches, 0, sizeof (Matches));
      memset (Completed, 0, sizeof (Completed));
      for (num = 0; Commands[num].name; num++)
        candidate (Completed, User_typed, Commands[num].name,
                   sizeof (Completed));
      Matches[Num_matched++] = User_typed;

      /* All matches are stored. Longest non-ambiguous string is ""
       * i.e. dont change 'buffer'. Fake successful return this time */
      if (User_typed[0] == 0)
        return 1;
    }

    if (Completed[0] == 0 && User_typed[0])
      return 0;

    /* Num_matched will _always_ be atleast 1 since the initial
     * user-typed string is always stored */
    if (numtabs == 1 && Num_matched == 2)
      snprintf (Completed, sizeof (Completed), "%s", Matches[0]);
    else if (numtabs > 1 && Num_matched > 2)
      /* cycle thru all the matches */
      snprintf (Completed, sizeof (Completed), "%s",
                Matches[(numtabs - 2) % Num_matched]);

    /* return the completed command */
    strncpy (buffer, Completed, len - spaces);
  }
  else if (!str_ncmp (buffer, "set", 3)
           || !str_ncmp (buffer, "unset", 5)
           || !str_ncmp (buffer, "reset", 5)
           || !str_ncmp (buffer, "toggle", 6)) {    /* complete variables */
    char *prefixes[] = { "no", "inv", "?", "&", 0 };

    pt++;
    /* loop through all the possible prefixes (no, inv, ...) */
    if (!str_ncmp (buffer, "set", 3)) {
      for (num = 0; prefixes[num]; num++) {
        if (!str_ncmp (pt, prefixes[num], str_len (prefixes[num]))) {
          pt += str_len (prefixes[num]);
          break;
        }
      }
    }

    /* first TAB. Collect all the matches */
    if (numtabs == 1) {
      Num_matched = 0;
      strfcpy (User_typed, pt, sizeof (User_typed));
      memset (Matches, 0, sizeof (Matches));
      memset (Completed, 0, sizeof (Completed));
      for (num = 0; MuttVars[num].option; num++)
        candidate (Completed, User_typed, MuttVars[num].option,
                   sizeof (Completed));
      Matches[Num_matched++] = User_typed;

      /* All matches are stored. Longest non-ambiguous string is ""
       * i.e. dont change 'buffer'. Fake successful return this time */
      if (User_typed[0] == 0)
        return 1;
    }

    if (Completed[0] == 0 && User_typed[0])
      return 0;

    /* Num_matched will _always_ be atleast 1 since the initial
     * user-typed string is always stored */
    if (numtabs == 1 && Num_matched == 2)
      snprintf (Completed, sizeof (Completed), "%s", Matches[0]);
    else if (numtabs > 1 && Num_matched > 2)
      /* cycle thru all the matches */
      snprintf (Completed, sizeof (Completed), "%s",
                Matches[(numtabs - 2) % Num_matched]);

    strncpy (pt, Completed, buffer + len - pt - spaces);
  }
  else if (!str_ncmp (buffer, "exec", 4)) {
    struct binding_t *menu = km_get_table (CurrentMenu);

    if (!menu && CurrentMenu != MENU_PAGER)
      menu = OpGeneric;

    pt++;
    /* first TAB. Collect all the matches */
    if (numtabs == 1) {
      Num_matched = 0;
      strfcpy (User_typed, pt, sizeof (User_typed));
      memset (Matches, 0, sizeof (Matches));
      memset (Completed, 0, sizeof (Completed));
      for (num = 0; menu[num].name; num++)
        candidate (Completed, User_typed, menu[num].name, sizeof (Completed));
      /* try the generic menu */
      if (Completed[0] == 0 && CurrentMenu != MENU_PAGER) {
        menu = OpGeneric;
        for (num = 0; menu[num].name; num++)
          candidate (Completed, User_typed, menu[num].name,
                     sizeof (Completed));
      }
      Matches[Num_matched++] = User_typed;

      /* All matches are stored. Longest non-ambiguous string is ""
       * i.e. dont change 'buffer'. Fake successful return this time */
      if (User_typed[0] == 0)
        return 1;
    }

    if (Completed[0] == 0 && User_typed[0])
      return 0;

    /* Num_matched will _always_ be atleast 1 since the initial
     * user-typed string is always stored */
    if (numtabs == 1 && Num_matched == 2)
      snprintf (Completed, sizeof (Completed), "%s", Matches[0]);
    else if (numtabs > 1 && Num_matched > 2)
      /* cycle thru all the matches */
      snprintf (Completed, sizeof (Completed), "%s",
                Matches[(numtabs - 2) % Num_matched]);

    strncpy (pt, Completed, buffer + len - pt - spaces);
  }
  else
    return 0;

  return 1;
}

int mutt_var_value_complete (char *buffer, size_t len, int pos)
{
  char var[STRING], *pt = buffer;
  int spaces;
  struct option_t* option = NULL;

  if (buffer[0] == 0)
    return 0;

  SKIPWS (buffer);
  spaces = buffer - pt;

  pt = buffer + pos - spaces;
  while ((pt > buffer) && !isspace ((unsigned char) *pt))
    pt--;
  pt++;                         /* move past the space */
  if (*pt == '=')               /* abort if no var before the '=' */
    return 0;

  if (str_ncmp (buffer, "set", 3) == 0) {
    strfcpy (var, pt, sizeof (var));
    /* ignore the trailing '=' when comparing */
    var[str_len (var) - 1] = 0;
    if (!(option = hash_find (ConfigOptions, var)))
      return 0;                 /* no such variable. */
    else {
      char tmp[LONG_STRING], tmp2[LONG_STRING];
      char *s, *d;
      size_t dlen = buffer + len - pt - spaces;
      char *vals[] = { "no", "yes", "ask-no", "ask-yes" };

      tmp[0] = '\0';

      if ((DTYPE (option->type) == DT_STR) ||
          (DTYPE (option->type) == DT_PATH) ||
          (DTYPE (option->type) == DT_RX)) {
        strfcpy (tmp, NONULL (*((char **) option->data)), sizeof (tmp));
        if (DTYPE (option->type) == DT_PATH)
          mutt_pretty_mailbox (tmp);
      }
      else if (DTYPE (option->type) == DT_ADDR) {
        rfc822_write_address (tmp, sizeof (tmp),
                              *((ADDRESS **) option->data), 0);
      }
      else if (DTYPE (option->type) == DT_QUAD)
        strfcpy (tmp, vals[quadoption (option->data)], sizeof (tmp));
      else if (DTYPE (option->type) == DT_NUM)
        snprintf (tmp, sizeof (tmp), "%d", (*((short *) option->data)));
      else if (DTYPE (option->type) == DT_SORT) {
        const struct mapping_t *map;
        char *p;

        switch (option->type & DT_SUBTYPE_MASK) {
        case DT_SORT_ALIAS:
          map = SortAliasMethods;
          break;
        case DT_SORT_BROWSER:
          map = SortBrowserMethods;
          break;
        case DT_SORT_KEYS:
          if ((WithCrypto & APPLICATION_PGP))
            map = SortKeyMethods;
          else
            map = SortMethods;
          break;
        default:
          map = SortMethods;
          break;
        }
        p =
          mutt_getnamebyvalue (*((short *) option->data) & SORT_MASK,
                               map);
        snprintf (tmp, sizeof (tmp), "%s%s%s",
                  (*((short *) option->data) & SORT_REVERSE) ?
                  "reverse-" : "",
                  (*((short *) option->data) & SORT_LAST) ? "last-" :
                  "", p);
      } 
      else if (DTYPE (option->type) == DT_MAGIC) {
        char *p;
        switch (DefaultMagic) {
          case M_MBOX:
            p = "mbox";
            break;
          case M_MMDF:
            p = "MMDF";
            break;
          case M_MH:
            p = "MH";
          break;
          case M_MAILDIR:
            p = "Maildir";
            break;
          default:
            p = "unknown";
        }
        strfcpy (tmp, p, sizeof (tmp));
      }
      else if (DTYPE (option->type) == DT_BOOL)
        strfcpy (tmp, option (option->data) ? "yes" : "no",
                 sizeof (tmp));
      else
        return 0;

      for (s = tmp, d = tmp2; *s && (d - tmp2) < sizeof (tmp2) - 2;) {
        if (*s == '\\' || *s == '"')
          *d++ = '\\';
        *d++ = *s++;
      }
      *d = '\0';

      strfcpy (tmp, pt, sizeof (tmp));
      snprintf (pt, dlen, "%s\"%s\"", tmp, tmp2);

      return 1;
    }
  }
  return 0;
}

/* Implement the -Q command line flag */
int mutt_query_variables (LIST * queries)
{
  LIST *p;

  char errbuff[STRING];
  char command[STRING];

  BUFFER err, token;

  memset (&err, 0, sizeof (err));
  memset (&token, 0, sizeof (token));

  err.data = errbuff;
  err.dsize = sizeof (errbuff);

  for (p = queries; p; p = p->next) {
    snprintf (command, sizeof (command), "set ?%s\n", p->data);
    if (mutt_parse_rc_line (command, &token, &err) == -1) {
      fprintf (stderr, "%s\n", err.data);
      mem_free (&token.data);
      return 1;
    }
    printf ("%s\n", err.data);
  }

  mem_free (&token.data);
  return 0;
}

char *mutt_getnamebyvalue (int val, const struct mapping_t *map)
{
  int i;

  for (i = 0; map[i].name; i++)
    if (map[i].value == val)
      return (map[i].name);
  return NULL;
}

int mutt_getvaluebyname (const char *name, const struct mapping_t *map)
{
  int i;

  for (i = 0; map[i].name; i++)
    if (ascii_strcasecmp (map[i].name, name) == 0)
      return (map[i].value);
  return (-1);
}

static int mutt_execute_commands (LIST * p)
{
  BUFFER err, token;
  char errstr[SHORT_STRING];

  memset (&err, 0, sizeof (err));
  err.data = errstr;
  err.dsize = sizeof (errstr);
  memset (&token, 0, sizeof (token));
  for (; p; p = p->next) {
    if (mutt_parse_rc_line (p->data, &token, &err) != 0) {
      fprintf (stderr, _("Error in command line: %s\n"), err.data);
      mem_free (&token.data);
      return (-1);
    }
  }
  mem_free (&token.data);
  return 0;
}

void mutt_init (int skip_sys_rc, LIST * commands)
{
  struct passwd *pw;
  struct utsname utsname;
  char *p, buffer[STRING], error[STRING];
  int i, default_rc = 0, need_pause = 0;
  BUFFER err;

  memset (&err, 0, sizeof (err));
  err.data = error;
  err.dsize = sizeof (error);

  /* use 3*sizeof(muttvars) instead of 2*sizeof() 
   * to have some room for $user_ vars */
  ConfigOptions = hash_create (sizeof (MuttVars) * 3);
  for (i = 0; MuttVars[i].option; i++) {
    if (DTYPE (MuttVars[i].type) != DT_SYS)
      hash_insert (ConfigOptions, MuttVars[i].option, &MuttVars[i], 0);
    else
      hash_insert (ConfigOptions, MuttVars[i].option,
                   add_option (MuttVars[i].option, MuttVars[i].init,
                               DT_SYS, 0), 0);
  }

  /* 
   * XXX - use something even more difficult to predict?
   */
  snprintf (AttachmentMarker, sizeof (AttachmentMarker),
            "\033]9;%ld\a", (long) time (NULL));

  /* on one of the systems I use, getcwd() does not return the same prefix
     as is listed in the passwd file */
  if ((p = getenv ("HOME")))
    Homedir = str_dup (p);

  /* Get some information about the user */
  if ((pw = getpwuid (getuid ()))) {
    char rnbuf[STRING];

    Username = str_dup (pw->pw_name);
    if (!Homedir)
      Homedir = str_dup (pw->pw_dir);

    Realname = str_dup (mutt_gecos_name (rnbuf, sizeof (rnbuf), pw));
    Shell = str_dup (pw->pw_shell);
    endpwent ();
  }
  else {
    if (!Homedir) {
      mutt_endwin (NULL);
      fputs (_("unable to determine home directory"), stderr);
      exit (1);
    }
    if ((p = getenv ("USER")))
      Username = str_dup (p);
    else {
      mutt_endwin (NULL);
      fputs (_("unable to determine username"), stderr);
      exit (1);
    }
    Shell = str_dup ((p = getenv ("SHELL")) ? p : "/bin/sh");
  }

  debug_start(Homedir);

  /* And about the host... */
  uname (&utsname);
  /* some systems report the FQDN instead of just the hostname */
  if ((p = strchr (utsname.nodename, '.'))) {
    Hostname = str_substrdup (utsname.nodename, p);
    p++;
    strfcpy (buffer, p, sizeof (buffer));       /* save the domain for below */
  }
  else
    Hostname = str_dup (utsname.nodename);

#ifndef DOMAIN
#define DOMAIN buffer
  if (!p && getdnsdomainname (buffer, sizeof (buffer)) == -1)
    Fqdn = str_dup ("@");
  else
#endif /* DOMAIN */
  if (*DOMAIN != '@') {
    Fqdn = mem_malloc (str_len (DOMAIN) + str_len (Hostname) + 2);
    sprintf (Fqdn, "%s.%s", NONULL (Hostname), DOMAIN); /* __SPRINTF_CHECKED__ */
  }
  else
    Fqdn = str_dup (NONULL (Hostname));

#ifdef USE_NNTP
  {
    FILE *f;
    char *i;

    if ((f = safe_fopen (SYSCONFDIR "/nntpserver", "r"))) {
      buffer[0] = '\0';
      fgets (buffer, sizeof (buffer), f);
      p = (char*) &buffer;
      SKIPWS (p);
      i = p;
      while (*i && (*i != ' ') && (*i != '\t') && (*i != '\r')
             && (*i != '\n'))
        i++;
      *i = '\0';
      NewsServer = str_dup (p);
      fclose (f);
    }
  }
  if ((p = getenv ("NNTPSERVER")))
    NewsServer = str_dup (p);
#endif

  if ((p = getenv ("MAIL")))
    Spoolfile = str_dup (p);
  else if ((p = getenv ("MAILDIR")))
    Spoolfile = str_dup (p);
  else {
#ifdef HOMESPOOL
    mutt_concat_path (buffer, NONULL (Homedir), MAILPATH, sizeof (buffer));
#else
    mutt_concat_path (buffer, MAILPATH, NONULL (Username), sizeof (buffer));
#endif
    Spoolfile = str_dup (buffer);
  }

  if ((p = getenv ("MAILCAPS")))
    MailcapPath = str_dup (p);
  else {
    /* Default search path from RFC1524 */
    MailcapPath =
      str_dup ("~/.mailcap:" PKGDATADIR "/mailcap:" SYSCONFDIR
                   "/mailcap:/etc/mailcap:/usr/etc/mailcap:/usr/local/etc/mailcap");
  }

  Tempdir = str_dup ((p = getenv ("TMPDIR")) ? p : "/tmp");

  p = getenv ("VISUAL");
  if (!p) {
    p = getenv ("EDITOR");
    if (!p)
      p = "vi";
  }
  Editor = str_dup (p);
  Visual = str_dup (p);

  if ((p = getenv ("REPLYTO")) != NULL) {
    BUFFER buf, token;

    snprintf (buffer, sizeof (buffer), "Reply-To: %s", p);

    memset (&buf, 0, sizeof (buf));
    buf.data = buf.dptr = buffer;
    buf.dsize = str_len (buffer);

    memset (&token, 0, sizeof (token));
    parse_my_hdr (&token, &buf, 0, &err);
    mem_free (&token.data);
  }

  if ((p = getenv ("EMAIL")) != NULL)
    From = rfc822_parse_adrlist (NULL, p);

  mutt_set_langinfo_charset ();
  mutt_set_charset (Charset);


  /* Set standard defaults */
  hash_map (ConfigOptions, mutt_set_default, 0);
  hash_map (ConfigOptions, mutt_restore_default, 0);

  CurrentMenu = MENU_MAIN;


#ifndef LOCALES_HACK
  /* Do we have a locale definition? */
  if (((p = getenv ("LC_ALL")) != NULL && p[0]) ||
      ((p = getenv ("LANG")) != NULL && p[0]) ||
      ((p = getenv ("LC_CTYPE")) != NULL && p[0]))
    set_option (OPTLOCALES);
#endif

#ifdef HAVE_GETSID
  /* Unset suspend by default if we're the session leader */
  if (getsid (0) == getpid ())
    unset_option (OPTSUSPEND);
#endif

  mutt_init_history ();




  /*
   * 
   *                       BIG FAT WARNING
   * 
   * When changing the code which looks for a configuration file,
   * please also change the corresponding code in muttbug.sh.in.
   * 
   * 
   */




  if (!Muttrc) {
#if 0
    snprintf (buffer, sizeof (buffer), "%s/.muttngrc-%s", NONULL (Homedir),
              MUTT_VERSION);
    if (access (buffer, F_OK) == -1)
#endif
      snprintf (buffer, sizeof (buffer), "%s/.muttngrc", NONULL (Homedir));
    if (access (buffer, F_OK) == -1)
#if 0
      snprintf (buffer, sizeof (buffer), "%s/.muttng/muttngrc-%s",
                NONULL (Homedir), MUTT_VERSION);
    if (access (buffer, F_OK) == -1)
#endif
      snprintf (buffer, sizeof (buffer), "%s/.muttng/muttngrc",
                NONULL (Homedir));

    default_rc = 1;
    Muttrc = str_dup (buffer);
  }
  else {
    strfcpy (buffer, Muttrc, sizeof (buffer));
    mem_free (&Muttrc);
    mutt_expand_path (buffer, sizeof (buffer));
    Muttrc = str_dup (buffer);
  }
  mem_free (&AliasFile);
  AliasFile = str_dup (NONULL (Muttrc));

  /* Process the global rc file if it exists and the user hasn't explicity
     requested not to via "-n".  */
  if (!skip_sys_rc) {
    snprintf (buffer, sizeof (buffer), "%s/Muttngrc-%s", SYSCONFDIR,
              MUTT_VERSION);
    if (access (buffer, F_OK) == -1)
      snprintf (buffer, sizeof (buffer), "%s/Muttngrc", SYSCONFDIR);
    if (access (buffer, F_OK) == -1)
      snprintf (buffer, sizeof (buffer), "%s/Muttngrc-%s", PKGDATADIR,
                MUTT_VERSION);
    if (access (buffer, F_OK) == -1)
      snprintf (buffer, sizeof (buffer), "%s/Muttngrc", PKGDATADIR);
    if (access (buffer, F_OK) != -1) {
      if (source_rc (buffer, &err) != 0) {
        fputs (err.data, stderr);
        fputc ('\n', stderr);
        need_pause = 1;
      }
    }
  }

  /* Read the user's initialization file.  */
  if (access (Muttrc, F_OK) != -1) {
    if (!option (OPTNOCURSES))
      mutt_endwin (NULL);
    if (source_rc (Muttrc, &err) != 0) {
      fputs (err.data, stderr);
      fputc ('\n', stderr);
      need_pause = 1;
    }
  }
  else if (!default_rc) {
    /* file specified by -F does not exist */
    snprintf (buffer, sizeof (buffer), "%s: %s", Muttrc, strerror (errno));
    mutt_endwin (buffer);
    exit (1);
  }

  if (mutt_execute_commands (commands) != 0)
    need_pause = 1;

  /* warn about synonym variables */
  if (!list_empty(Synonyms)) {
    int i = 0;
    fprintf (stderr, _("Warning: the following synonym variables were found:\n"));
    for (i = 0; i < Synonyms->length; i++) {
      struct option_t* newopt = NULL, *oldopt = NULL;
      newopt = (struct option_t*) ((syn_t*) Synonyms->data[i])->n;
      oldopt = (struct option_t*) ((syn_t*) Synonyms->data[i])->o;
      fprintf (stderr, "$%s ($%s should be used) (%s:%d)\n",
               oldopt ? NONULL (oldopt->option) : "",
               newopt ? NONULL (newopt->option) : "",
               NONULL(((syn_t*) Synonyms->data[i])->f),
               ((syn_t*) Synonyms->data[i])->l);
    }
    fprintf (stderr, _("Warning: synonym variables are scheduled"
                       " for removal.\n"));
    list_del (&Synonyms, syn_del);
    need_pause = 1;
  }

  if (need_pause && !option (OPTNOCURSES)) {
    if (mutt_any_key_to_continue (NULL) == -1)
      mutt_exit (1);
  }

#if 0
  set_option (OPTWEED);         /* turn weeding on by default */
#endif
}

int mutt_get_hook_type (const char *name)
{
  struct command_t *c;

  for (c = Commands; c->name; c++)
    if (c->func == mutt_parse_hook && ascii_strcasecmp (c->name, name) == 0)
      return c->data;
  return 0;
}

/* compare two option_t*'s for sorting -t/-T output */
static int opt_cmp (const void* a, const void* b) {
  return (str_cmp ((*(struct option_t**) a)->option,
                       (*(struct option_t**) b)->option));
}

/* callback for hash_map() to put all non-synonym vars into list */
static void opt_sel_full (const char* key, void* data,
                          unsigned long more) {
  list2_t** l = (list2_t**) more;
  struct option_t* option = (struct option_t*) data;

  if (DTYPE (option->type) == DT_SYN)
    return;
  list_push_back (l, option);
}

/* callback for hash_map() to put all changed non-synonym vars into list */
static void opt_sel_diff (const char* key, void* data,
                          unsigned long more) {
  list2_t** l = (list2_t**) more;
  struct option_t* option = (struct option_t*) data;
  char buf[LONG_STRING];

  if (DTYPE (option->type) == DT_SYN)
    return;

  mutt_option_value (option->option, buf, sizeof (buf));
  if (str_cmp (buf, option->init) != 0)
    list_push_back (l, option);
}

/* dump out the value of all the variables we have */
int mutt_dump_variables (int full) {
  int i = 0;
  char outbuf[STRING];
  list2_t* tmp = NULL;
  struct option_t* option = NULL;

  /* get all non-synonyms into list... */
  hash_map (ConfigOptions, full ? opt_sel_full : opt_sel_diff,
            (unsigned long) &tmp);

  if (!list_empty(tmp)) {
    /* ...and dump list sorted */
    qsort (tmp->data, tmp->length, sizeof (void*), opt_cmp);
    for (i = 0; i < tmp->length; i++) {
      option = (struct option_t*) tmp->data[i];
      FuncTable[DTYPE (option->type)].opt_to_string
        (outbuf, sizeof (outbuf), option);
      printf ("%s\n", outbuf);
    }
  }
  list_del (&tmp, NULL);
  return 0;
}
