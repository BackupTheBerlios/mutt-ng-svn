/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2002 Michael R. Elkins <me@mutt.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <string.h>
#include <ctype.h>

#include "lib/mem.h"
#include "lib/intl.h"
#include "lib/str.h"
#include "lib/rx.h"
#include "lib/debug.h"

#include "mutt.h"
#include "mutt_curses.h"
#include "mutt_idna.h"
#include "mutt_menu.h"
#include "mapping.h"
#include "sort.h"

#define RSORT(x) (SortAlias & SORT_REVERSE) ? -x : x

static struct mapping_t AliasHelp[] = {
  {N_("Exit"), OP_EXIT},
  {N_("Del"), OP_DELETE},
  {N_("Undel"), OP_UNDELETE},
  {N_("Select"), OP_GENERIC_SELECT_ENTRY},
  {N_("Help"), OP_HELP},
  {NULL}
};

ADDRESS *mutt_lookup_alias (const char *s)
{
  ALIAS *t = Aliases;

  for (; t; t = t->next)
    if (!str_casecmp (s, t->name))
      return (t->addr);
  return (NULL);                /* no such alias */
}

static ADDRESS *mutt_expand_aliases_r (ADDRESS * a, LIST ** expn)
{
  ADDRESS *head = NULL, *last = NULL, *t, *w;
  LIST *u;
  char i;
  const char *fqdn;

  while (a) {
    if (!a->group && !a->personal && a->mailbox
        && strchr (a->mailbox, '@') == NULL) {
      t = mutt_lookup_alias (a->mailbox);

      if (t) {
        i = 0;
        for (u = *expn; u; u = u->next) {
          if (str_cmp (a->mailbox, u->data) == 0) { /* alias already found */
            debug_print(1, ("loop in alias found for '%s'\n", a->mailbox));
            i = 1;
            break;
          }
        }

        if (!i) {
          u = mem_malloc (sizeof (LIST));
          u->data = str_dup (a->mailbox);
          u->next = *expn;
          *expn = u;
          w = rfc822_cpy_adr (t);
          w = mutt_expand_aliases_r (w, expn);
          if (head)
            last->next = w;
          else
            head = last = w;
          while (last && last->next)
            last = last->next;
        }
        t = a;
        a = a->next;
        t->next = NULL;
        rfc822_free_address (&t);
        continue;
      }
      else {
        struct passwd *pw = getpwnam (a->mailbox);

        if (pw) {
          char namebuf[STRING];

          mutt_gecos_name (namebuf, sizeof (namebuf), pw);
          str_replace (&a->personal, namebuf);
        }
      }
    }

    if (head) {
      last->next = a;
      last = last->next;
    }
    else
      head = last = a;
    a = a->next;
    last->next = NULL;
  }

  if (option (OPTUSEDOMAIN) && (fqdn = mutt_fqdn (1))) {
    /* now qualify all local addresses */
    rfc822_qualify (head, fqdn);
  }

  return (head);
}

ADDRESS *mutt_expand_aliases (ADDRESS * a)
{
  ADDRESS *t;
  LIST *expn = NULL;            /* previously expanded aliases to avoid loops */

  t = mutt_expand_aliases_r (a, &expn);
  mutt_free_list (&expn);
  return (mutt_remove_duplicates (t));
}

void mutt_expand_aliases_env (ENVELOPE * env)
{
  env->from = mutt_expand_aliases (env->from);
  env->to = mutt_expand_aliases (env->to);
  env->cc = mutt_expand_aliases (env->cc);
  env->bcc = mutt_expand_aliases (env->bcc);
  env->reply_to = mutt_expand_aliases (env->reply_to);
  env->mail_followup_to = mutt_expand_aliases (env->mail_followup_to);
}


/* 
 * if someone has an address like
 *	From: Michael `/bin/rm -f ~` Elkins <me@mutt.org>
 * and the user creates an alias for this, Mutt could wind up executing
 * the backtics because it writes aliases like
 *	alias me Michael `/bin/rm -f ~` Elkins <me@mutt.org>
 * To avoid this problem, use a backslash (\) to quote any backtics.  We also
 * need to quote backslashes as well, since you could defeat the above by
 * doing
 *	From: Michael \`/bin/rm -f ~\` Elkins <me@mutt.org>
 * since that would get aliased as
 *	alias me Michael \\`/bin/rm -f ~\\` Elkins <me@mutt.org>
 * which still gets evaluated because the double backslash is not a quote.
 * 
 * Additionally, we need to quote ' and " characters - otherwise, mutt will
 * interpret them on the wrong parsing step.
 * 
 * $ wants to be quoted since it may indicate the start of an environment
 * variable.
 */

static void write_safe_address (FILE * fp, char *s)
{
  while (*s) {
    if (*s == '\\' || *s == '`' || *s == '\'' || *s == '"' || *s == '$')
      fputc ('\\', fp);
    fputc (*s, fp);
    s++;
  }
}

ADDRESS *mutt_get_address (ENVELOPE * env, char **pfxp)
{
  ADDRESS *adr;
  char *pfx = NULL;

  if (mutt_addr_is_user (env->from)) {
    if (env->to && !mutt_is_mail_list (env->to)) {
      pfx = "To";
      adr = env->to;
    }
    else {
      pfx = "Cc";
      adr = env->cc;
    }
  }
  else if (env->reply_to && !mutt_is_mail_list (env->reply_to)) {
    pfx = "Reply-To";
    adr = env->reply_to;
  }
  else {
    adr = env->from;
    pfx = "From";
  }

  if (pfxp)
    *pfxp = pfx;

  return adr;
}

void mutt_create_alias (ENVELOPE * cur, ADDRESS * iadr)
{
  ALIAS *new, *t;
  char buf[LONG_STRING], prompt[SHORT_STRING], *pc;
  char *err = NULL;
  char fixed[LONG_STRING];
  FILE *rc;
  ADDRESS *adr = NULL;

  if (cur) {
    adr = mutt_get_address (cur, NULL);
  }
  else if (iadr) {
    adr = iadr;
  }

  if (adr && adr->mailbox) {
    strfcpy (buf, adr->mailbox, sizeof (buf));
    if ((pc = strchr (buf, '@')))
      *pc = 0;
  }
  else
    buf[0] = '\0';

  /* Don't suggest a bad alias name in the event of a strange local part. */
  mutt_check_alias_name (buf, buf);

retry_name:
  /* add a new alias */
  if (mutt_get_field (_("Alias as: "), buf, sizeof (buf), 0) != 0 || !buf[0])
    return;

  /* check to see if the user already has an alias defined */
  if (mutt_lookup_alias (buf)) {
    mutt_error _("You already have an alias defined with that name!");

    return;
  }

  if (mutt_check_alias_name (buf, fixed)) {
    switch (mutt_yesorno
            (_("Warning: This alias name may not work.  Fix it?"), M_YES)) {
    case M_YES:
      strfcpy (buf, fixed, sizeof (buf));
      goto retry_name;
    case -1:
      return;
    }
  }

  new = mem_calloc (1, sizeof (ALIAS));
  new->self = new;
  new->name = str_dup (buf);

  mutt_addrlist_to_local (adr);

  if (adr)
    strfcpy (buf, adr->mailbox, sizeof (buf));
  else
    buf[0] = 0;

  mutt_addrlist_to_idna (adr, NULL);

  do {
    if (mutt_get_field (_("Address: "), buf, sizeof (buf), 0) != 0 || !buf[0]) {
      mutt_free_alias (&new);
      return;
    }

    if ((new->addr = rfc822_parse_adrlist (new->addr, buf)) == NULL)
      BEEP ();
    if (mutt_addrlist_to_idna (new->addr, &err)) {
      mutt_error (_("Error: '%s' is a bad IDN."), err);
      mutt_sleep (2);
      continue;
    }
  }
  while (new->addr == NULL);

  if (adr && adr->personal && !mutt_is_mail_list (adr))
    strfcpy (buf, adr->personal, sizeof (buf));
  else
    buf[0] = 0;

  if (mutt_get_field (_("Personal name: "), buf, sizeof (buf), 0) != 0) {
    mutt_free_alias (&new);
    return;
  }
  new->addr->personal = str_dup (buf);

  buf[0] = 0;
  rfc822_write_address (buf, sizeof (buf), new->addr, 1);
  snprintf (prompt, sizeof (prompt), _("[%s = %s] Accept?"), new->name, buf);
  if (mutt_yesorno (prompt, M_YES) != M_YES) {
    mutt_free_alias (&new);
    return;
  }

  if ((t = Aliases)) {
    while (t->next)
      t = t->next;
    t->next = new;
  }
  else
    Aliases = new;

  strfcpy (buf, NONULL (AliasFile), sizeof (buf));
  if (mutt_get_field (_("Save to file: "), buf, sizeof (buf), M_FILE) != 0)
    return;
  mutt_expand_path (buf, sizeof (buf));
  if ((rc = safe_fopen (buf, "a"))) {
    if (mutt_check_alias_name (new->name, NULL))
      mutt_quote_filename (buf, sizeof (buf), new->name);
    else
      strfcpy (buf, new->name, sizeof (buf));
    fprintf (rc, "alias %s ", buf);
    buf[0] = 0;
    rfc822_write_address (buf, sizeof (buf), new->addr, 0);
    write_safe_address (rc, buf);
    fputc ('\n', rc);
    fclose (rc);
    mutt_message _("Alias added.");
  }
  else
    mutt_perror (buf);
}

/* 
 * Sanity-check an alias name:  Only characters which are non-special to both
 * the RFC 822 and the mutt configuration parser are permitted.
 */

static int check_alias_name_char (char c)
{
  return (c == '-' || c == '_' || c == '+' || c == '=' || c == '.' ||
          isalnum ((unsigned char) c));
}

int mutt_check_alias_name (const char *s, char *d)
{
  int rv = 0;

  for (; *s; s++) {
    if (!check_alias_name_char (*s)) {
      if (!d)
        return -1;
      else {
        *d++ = '_';
        rv = -1;
      }
    }
    else if (d)
      *d++ = *s;
  }
  if (d)
    *d++ = *s;
  return rv;
}

/*
 * This routine looks to see if the user has an alias defined for the given
 * address.
 */
ADDRESS *alias_reverse_lookup (ADDRESS * a)
{
  ALIAS *t = Aliases;
  ADDRESS *ap;

  if (!a || !a->mailbox)
    return NULL;

  for (; t; t = t->next) {
    /* cycle through all addresses if this is a group alias */
    for (ap = t->addr; ap; ap = ap->next) {
      if (!ap->group && ap->mailbox &&
          ascii_strcasecmp (ap->mailbox, a->mailbox) == 0)
        return ap;
    }
  }
  return 0;
}

/* alias_complete() -- alias completion routine
 *
 * given a partial alias, this routine attempts to fill in the alias
 * from the alias list as much as possible. if given empty search string
 * or found nothing, present all aliases
 */
int mutt_alias_complete (char *s, size_t buflen)
{
  ALIAS *a = Aliases;
  ALIAS *a_list = NULL, *a_cur = NULL;
  char bestname[HUGE_STRING];
  int i;

#define min(a,b)        ((a<b)?a:b)

  if (s[0] != 0) {              /* avoid empty string as strstr argument */
    memset (bestname, 0, sizeof (bestname));

    while (a) {
      if (a->name && strstr (a->name, s) == a->name) {
        if (!bestname[0])       /* init */
          strfcpy (bestname, a->name,
                   min (str_len (a->name) + 1, sizeof (bestname)));
        else {
          for (i = 0; a->name[i] && a->name[i] == bestname[i]; i++);
          bestname[i] = 0;
        }
      }
      a = a->next;
    }

    if (bestname[0] != 0) {
      if (str_cmp (bestname, s) != 0) {
        /* we are adding something to the completion */
        strfcpy (s, bestname, str_len (bestname) + 1);
        return 1;
      }

      /* build alias list and show it */

      a = Aliases;
      while (a) {
        if (a->name && (strstr (a->name, s) == a->name)) {
          if (!a_list)          /* init */
            a_cur = a_list = (ALIAS *) mem_malloc (sizeof (ALIAS));
          else {
            a_cur->next = (ALIAS *) mem_malloc (sizeof (ALIAS));
            a_cur = a_cur->next;
          }
          memcpy (a_cur, a, sizeof (ALIAS));
          a_cur->next = NULL;
        }
        a = a->next;
      }
    }
  }

  bestname[0] = 0;
  mutt_alias_menu (bestname, sizeof (bestname), a_list ? a_list : Aliases);
  if (bestname[0] != 0)
    strfcpy (s, bestname, buflen);

  /* free the alias list */
  while (a_list) {
    a_cur = a_list;
    a_list = a_list->next;
    mem_free (&a_cur);
  }

  /* remove any aliases marked for deletion */
  a_list = NULL;
  for (a_cur = Aliases; a_cur;) {
    if (a_cur->del) {
      if (a_list)
        a_list->next = a_cur->next;
      else
        Aliases = a_cur->next;

      a_cur->next = NULL;
      mutt_free_alias (&a_cur);

      if (a_list)
        a_cur = a_list;
      else
        a_cur = Aliases;
    }
    else {
      a_list = a_cur;
      a_cur = a_cur->next;
    }
  }

  return 0;
}

static int string_is_address (const char *str, const char *u, const char *d)
{
  char buf[LONG_STRING];

  snprintf (buf, sizeof (buf), "%s@%s", NONULL (u), NONULL (d));
  if (ascii_strcasecmp (str, buf) == 0)
    return 1;

  return 0;
}

/* returns TRUE if the given address belongs to the user. */
int mutt_addr_is_user (ADDRESS * addr)
{
  /* NULL address is assumed to be the user. */
  if (!addr) {
    debug_print(5, ("yes, NULL address\n"));
    return 1;
  }
  if (!addr->mailbox) {
    debug_print(5, ("no, no mailbox\n"));
    return 0;
  }

  if (ascii_strcasecmp (addr->mailbox, Username) == 0) {
    debug_print(5, ("yes, %s = %s\n", addr->mailbox, Username));
    return 1;
  }
  if (string_is_address (addr->mailbox, Username, Hostname)) {
    debug_print(5, ("yes, %s = %s @ %s \n", addr->mailbox, Username, Hostname));
    return 1;
  }
  if (string_is_address (addr->mailbox, Username, mutt_fqdn (0))) {
    debug_print(5, ("yes, %s = %s @ %s \n", addr->mailbox, Username, mutt_fqdn (0)));
    return 1;
  }
  if (string_is_address (addr->mailbox, Username, mutt_fqdn (1))) {
    debug_print(5, ("yes, %s = %s @ %s \n", addr->mailbox, Username, mutt_fqdn (1)));
    return 1;
  }

  if (From && !ascii_strcasecmp (From->mailbox, addr->mailbox)) {
    debug_print(5, ("yes, %s = %s\n", addr->mailbox, From->mailbox));
    return 1;
  }

  if (rx_list_match (Alternates, addr->mailbox)) {
    debug_print(5, ("yes, %s matched by alternates.\n", addr->mailbox));
    if (rx_list_match (UnAlternates, addr->mailbox))
      debug_print(5, ("but, %s matched by unalternates.\n", addr->mailbox));
    else
      return 1;
  }

  debug_print(5, ("no, all failed.\n"));
  return 0;
}

static const char *alias_format_str (char *dest, size_t destlen, char op,
                                     const char *src, const char *fmt,
                                     const char *ifstring,
                                     const char *elsestring,
                                     unsigned long data, format_flag flags)
{
  char tmp[SHORT_STRING], adr[SHORT_STRING];
  ALIAS *alias = (ALIAS *) data;

  switch (op) {
  case 'f':
    snprintf (tmp, sizeof (tmp), "%%%ss", fmt);
    snprintf (dest, destlen, tmp, alias->del ? "D" : " ");
    break;
  case 'a':
    mutt_format_s (dest, destlen, fmt, alias->name);
    break;
  case 'r':
    adr[0] = 0;
    rfc822_write_address (adr, sizeof (adr), alias->addr, 1);
    snprintf (tmp, sizeof (tmp), "%%%ss", fmt);
    snprintf (dest, destlen, tmp, adr);
    break;
  case 'n':
    snprintf (tmp, sizeof (tmp), "%%%sd", fmt);
    snprintf (dest, destlen, tmp, alias->num + 1);
    break;
  case 't':
    dest[0] = alias->tagged ? '*' : ' ';
    dest[1] = 0;
    break;
  }

  return (src);
}

static void alias_entry (char *s, size_t slen, MUTTMENU * m, int num)
{
  mutt_FormatString (s, slen, NONULL (AliasFmt), alias_format_str,
                     (unsigned long) ((ALIAS **) m->data)[num],
                     M_FORMAT_ARROWCURSOR);
}

static int alias_tag (MUTTMENU * menu, int n, int m)
{
  ALIAS *cur = ((ALIAS **) menu->data)[n];
  int ot = cur->tagged;

  cur->tagged = (m >= 0 ? m : !cur->tagged);

  return cur->tagged - ot;
}

static int alias_SortAlias (const void *a, const void *b)
{
  ALIAS *pa = *(ALIAS **) a;
  ALIAS *pb = *(ALIAS **) b;
  int r = str_casecmp (pa->name, pb->name);

  return (RSORT (r));
}

static int alias_SortAddress (const void *a, const void *b)
{
  ADDRESS *pa = (*(ALIAS **) a)->addr;
  ADDRESS *pb = (*(ALIAS **) b)->addr;
  int r;

  if (pa == pb)
    r = 0;
  else if (pa == NULL)
    r = -1;
  else if (pb == NULL)
    r = 1;
  else if (pa->personal) {
    if (pb->personal)
      r = str_casecmp (pa->personal, pb->personal);
    else
      r = 1;
  }
  else if (pb->personal)
    r = -1;
  else
    r = ascii_strcasecmp (pa->mailbox, pb->mailbox);
  return (RSORT (r));
}

void mutt_alias_menu (char *buf, size_t buflen, ALIAS * aliases)
{
  ALIAS *aliasp;
  MUTTMENU *menu;
  ALIAS **AliasTable = NULL;
  int t = -1;
  int i, done = 0;
  int op;
  char helpstr[SHORT_STRING];

  int omax;

  if (!aliases) {
    mutt_error _("You have no aliases!");

    return;
  }

  /* tell whoever called me to redraw the screen when I return */
  set_option (OPTNEEDREDRAW);

  menu = mutt_new_menu ();
  menu->make_entry = alias_entry;
  menu->tag = alias_tag;
  menu->menu = MENU_ALIAS;
  menu->title = _("Aliases");
  menu->help =
    mutt_compile_help (helpstr, sizeof (helpstr), MENU_ALIAS, AliasHelp);

new_aliases:

  omax = menu->max;

  /* count the number of aliases */
  for (aliasp = aliases; aliasp; aliasp = aliasp->next) {
    aliasp->self->del = 0;
    aliasp->self->tagged = 0;
    menu->max++;
  }

  mem_realloc (&AliasTable, menu->max * sizeof (ALIAS *));
  menu->data = AliasTable;

  for (i = omax, aliasp = aliases; aliasp; aliasp = aliasp->next, i++) {
    AliasTable[i] = aliasp->self;
    aliases = aliasp;
  }

  if ((SortAlias & SORT_MASK) != SORT_ORDER) {
    qsort (AliasTable, i, sizeof (ALIAS *),
           (SortAlias & SORT_MASK) ==
           SORT_ADDRESS ? alias_SortAddress : alias_SortAlias);
  }

  for (i = 0; i < menu->max; i++)
    AliasTable[i]->num = i;

  while (!done) {
    if (aliases->next) {
      menu->redraw |= REDRAW_FULL;
      aliases = aliases->next;
      goto new_aliases;
    }

    switch ((op = mutt_menuLoop (menu))) {
    case OP_DELETE:
    case OP_UNDELETE:
      if (menu->tagprefix) {
        for (i = 0; i < menu->max; i++)
          if (AliasTable[i]->tagged)
            AliasTable[i]->del = (op == OP_DELETE) ? 1 : 0;
        menu->redraw |= REDRAW_INDEX;
      }
      else {
        AliasTable[menu->current]->self->del = (op == OP_DELETE) ? 1 : 0;
        menu->redraw |= REDRAW_CURRENT;
        if (option (OPTRESOLVE) && menu->current < menu->max - 1) {
          menu->current++;
          menu->redraw |= REDRAW_INDEX;
        }
      }
      break;
    case OP_GENERIC_SELECT_ENTRY:
      t = menu->current;
    case OP_EXIT:
      done = 1;
      break;
    }
  }

  for (i = 0; i < menu->max; i++) {
    if (AliasTable[i]->tagged) {
      mutt_addrlist_to_local (AliasTable[i]->addr);
      rfc822_write_address (buf, buflen, AliasTable[i]->addr, 0);
      t = -1;
    }
  }

  if (t != -1) {
    mutt_addrlist_to_local (AliasTable[t]->addr);
    rfc822_write_address (buf, buflen, AliasTable[t]->addr, 0);
  }

  mutt_menuDestroy (&menu);
  mem_free (&AliasTable);

}
