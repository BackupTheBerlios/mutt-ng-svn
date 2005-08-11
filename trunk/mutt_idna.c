/*
 * Copyright notice from original mutt:
 * Copyright (C) 2003 Thomas Roessler <roessler@does-not-exist.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "mutt.h"
#include "ascii.h"
#include "charset.h"
#include "mutt_idna.h"

#include "lib/mem.h"
#include "lib/intl.h"
#include "lib/str.h"
#include "lib/debug.h"

/* The low-level interface we use. */

#ifndef HAVE_LIBIDN

int mutt_idna_to_local (const char *in, char **out, int flags)
{
  *out = str_dup (in);
  return 1;
}

int mutt_local_to_idna (const char *in, char **out)
{
  *out = str_dup (in);
  return 0;
}

#else

int mutt_idna_to_local (const char *in, char **out, int flags)
{
  *out = NULL;

  if (!option (OPTUSEIDN))
    goto notrans;

  if (!in)
    goto notrans;

  /* Is this the right function?  Interesting effects with some bad identifiers! */
  if (idna_to_unicode_8z8z (in, out, 1) != IDNA_SUCCESS)
    goto notrans;
  if (mutt_convert_string (out, "utf-8", Charset, M_ICONV_HOOK_TO) == -1)
    goto notrans;

  /* 
   * make sure that we can convert back and come out with the same
   * domain name. 
   */

  if ((flags & MI_MAY_BE_IRREVERSIBLE) == 0) {
    int irrev = 0;
    char *t2 = NULL;
    char *tmp = str_dup (*out);

    if (mutt_convert_string (&tmp, Charset, "utf-8", M_ICONV_HOOK_FROM) == -1)
      irrev = 1;
    if (!irrev && idna_to_ascii_8z (tmp, &t2, 1) != IDNA_SUCCESS)
      irrev = 1;
    if (!irrev && ascii_strcasecmp (t2, in)) {
      debug_print (1, ("not reversible. in = '%s', t2 = '%s'.\n", in, t2));
      irrev = 1;
    }

    mem_free (&t2);
    mem_free (&tmp);

    if (irrev)
      goto notrans;
  }

  return 0;

notrans:
  mem_free (out);
  *out = str_dup (in);
  return 1;
}

int mutt_local_to_idna (const char *in, char **out)
{
  int rv = 0;
  char *tmp = str_dup (in);

  *out = NULL;

  if (!in) {
    *out = NULL;
    return -1;
  }

  if (mutt_convert_string (&tmp, Charset, "utf-8", M_ICONV_HOOK_FROM) == -1)
    rv = -1;
  if (!rv && idna_to_ascii_8z (tmp, out, 1) != IDNA_SUCCESS)
    rv = -2;

  mem_free (&tmp);
  if (rv < 0) {
    mem_free (out);
    *out = str_dup (in);
  }
  return rv;
}

#endif


/* higher level functions */

static int mbox_to_udomain (const char *mbx, char **user, char **domain)
{
  char *p;

  *user = NULL;
  *domain = NULL;

  p = strchr (mbx, '@');
  if (!p)
    return -1;
  *user = mem_calloc ((p - mbx + 1), sizeof (mbx[0]));
  strfcpy (*user, mbx, (p - mbx + 1));
  *domain = str_dup (p + 1);
  return 0;
}

int mutt_addrlist_to_idna (ADDRESS * a, char **err)
{
  char *user = NULL, *domain = NULL;
  char *tmp = NULL;
  int e = 0;

  if (err)
    *err = NULL;

  for (; a; a = a->next) {
    if (!a->mailbox)
      continue;
    if (mbox_to_udomain (a->mailbox, &user, &domain) == -1)
      continue;

    if (mutt_local_to_idna (domain, &tmp) < 0) {
      e = 1;
      if (err)
        *err = str_dup (domain);
    }
    else {
      mem_realloc (&a->mailbox, str_len (user) + str_len (tmp) + 2);
      sprintf (a->mailbox, "%s@%s", NONULL (user), NONULL (tmp));       /* __SPRINTF_CHECKED__ */
    }

    mem_free (&domain);
    mem_free (&user);
    mem_free (&tmp);

    if (e)
      return -1;
  }

  return 0;
}

int mutt_addrlist_to_local (ADDRESS * a)
{
  char *user, *domain;
  char *tmp = NULL;

  for (; a; a = a->next) {
    if (!a->mailbox)
      continue;
    if (mbox_to_udomain (a->mailbox, &user, &domain) == -1)
      continue;

    if (mutt_idna_to_local (domain, &tmp, 0) == 0) {
      mem_realloc (&a->mailbox, str_len (user) + str_len (tmp) + 2);
      sprintf (a->mailbox, "%s@%s", NONULL (user), NONULL (tmp));       /* __SPRINTF_CHECKED__ */
    }

    mem_free (&domain);
    mem_free (&user);
    mem_free (&tmp);
  }

  return 0;
}

/* convert just for displaying purposes */
const char *mutt_addr_for_display (ADDRESS * a)
{
  static char *buff = NULL;
  char *tmp = NULL;

  /* user and domain will be either allocated or reseted to the NULL in
   * the mbox_to_udomain(), but for safety... */
  char *domain = NULL;
  char *user = NULL;

  mem_free (&buff);

  if (mbox_to_udomain (a->mailbox, &user, &domain) != 0)
    return a->mailbox;
  if (mutt_idna_to_local (domain, &tmp, MI_MAY_BE_IRREVERSIBLE) != 0) {
    mem_free (&user);
    mem_free (&domain);
    mem_free (&tmp);
    return a->mailbox;
  }

  mem_realloc (&buff, str_len (tmp) + str_len (user) + 2);
  sprintf (buff, "%s@%s", NONULL (user), NONULL (tmp)); /* __SPRINTF_CHECKED__ */
  mem_free (&tmp);
  mem_free (&user);
  mem_free (&domain);
  return buff;
}

/* Convert an ENVELOPE structure */

void mutt_env_to_local (ENVELOPE * e)
{
  mutt_addrlist_to_local (e->return_path);
  mutt_addrlist_to_local (e->from);
  mutt_addrlist_to_local (e->to);
  mutt_addrlist_to_local (e->cc);
  mutt_addrlist_to_local (e->bcc);
  mutt_addrlist_to_local (e->reply_to);
  mutt_addrlist_to_local (e->mail_followup_to);
}

/* Note that `a' in the `env->a' expression is macro argument, not
 * "real" name of an `env' compound member.  Real name will be substituted
 * by preprocessor at the macro-expansion time.
 */
#define H_TO_IDNA(a)	\
  if (mutt_addrlist_to_idna (env->a, err) && !e) \
  { \
     if (tag) *tag = #a; e = 1; err = NULL; \
  }

int mutt_env_to_idna (ENVELOPE * env, char **tag, char **err)
{
  int e = 0;

  H_TO_IDNA (return_path);
  H_TO_IDNA (from);
  H_TO_IDNA (to);
  H_TO_IDNA (cc);
  H_TO_IDNA (bcc);
  H_TO_IDNA (reply_to);
  H_TO_IDNA (mail_followup_to);
  return e;
}

#undef H_TO_IDNA
