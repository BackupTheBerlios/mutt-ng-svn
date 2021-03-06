/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2002 Michael R. Elkins <me@mutt.org>, and others
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
#include "mx.h"
#include "mutt_crypt.h"

#ifdef USE_COMPRESSED
#include "compress.h"
#endif

#include "lib/mem.h"
#include "lib/intl.h"
#include "lib/str.h"
#include "lib/rx.h"

#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#define ERROR_STOP      0

typedef struct hook {
  int type;                     /* hook type */
  rx_t rx;                      /* regular expression */
  char *command;                /* filename, command or pattern to execute */
  pattern_t *pattern;           /* used for fcc,save,send-hook */
  struct hook *next;
} HOOK;

static HOOK *Hooks = NULL;

static int current_hook_type = 0;

int mutt_parse_hook (BUFFER * buf, BUFFER * s, unsigned long data,
                     BUFFER * err)
{
  HOOK *ptr;
  BUFFER command, pattern;
  int rc, not = 0;
  regex_t *rx = NULL;
  pattern_t *pat = NULL;
  char path[_POSIX_PATH_MAX];

  memset (&pattern, 0, sizeof (pattern));
  memset (&command, 0, sizeof (command));

  if (*s->dptr == '!') {
    s->dptr++;
    SKIPWS (s->dptr);
    not = 1;
  }

  mutt_extract_token (&pattern, s, 0);

  if (!MoreArgs (s)) {
    strfcpy (err->data, _("too few arguments"), err->dsize);
    goto error;
  }

  mutt_extract_token (&command, s,
                      (data &
                       (M_FOLDERHOOK | M_SENDHOOK | M_SEND2HOOK |
                        M_ACCOUNTHOOK | M_REPLYHOOK)) ? M_TOKEN_SPACE : 0);

  if (!command.data) {
    strfcpy (err->data, _("too few arguments"), err->dsize);
    goto error;
  }

  if (MoreArgs (s)) {
    strfcpy (err->data, _("too many arguments"), err->dsize);
    goto error;
  }

  if (data & (M_FOLDERHOOK | M_MBOXHOOK)) {
    strfcpy (path, pattern.data, sizeof (path));
    _mutt_expand_path (path, sizeof (path), 1);
    mem_free (&pattern.data);
    memset (&pattern, 0, sizeof (pattern));
    pattern.data = str_dup (path);
  }
#ifdef USE_COMPRESSED
  else if (data & (M_APPENDHOOK | M_OPENHOOK | M_CLOSEHOOK)) {
    if (mutt_test_compress_command (command.data)) {
      strfcpy (err->data, _("bad formatted command string"), err->dsize);
      return (-1);
    }
  }
#endif
  else if (DefaultHook && !(data & (M_CHARSETHOOK | M_ACCOUNTHOOK))
           && (!WithCrypto || !(data & M_CRYPTHOOK))
    ) {
    char tmp[HUGE_STRING];

    strfcpy (tmp, pattern.data, sizeof (tmp));
    mutt_check_simple (tmp, sizeof (tmp), DefaultHook);
    mem_free (&pattern.data);
    memset (&pattern, 0, sizeof (pattern));
    pattern.data = str_dup (tmp);
  }

  if (data & (M_MBOXHOOK | M_SAVEHOOK | M_FCCHOOK)) {
    strfcpy (path, command.data, sizeof (path));
    mutt_expand_path (path, sizeof (path));
    mem_free (&command.data);
    memset (&command, 0, sizeof (command));
    command.data = str_dup (path);
  }

  /* check to make sure that a matching hook doesn't already exist */
  for (ptr = Hooks; ptr; ptr = ptr->next) {
    if (ptr->type == data &&
        ptr->rx.not == not && !str_cmp (pattern.data, ptr->rx.pattern)) {
      if (data &
          (M_FOLDERHOOK | M_SENDHOOK | M_SEND2HOOK | M_MESSAGEHOOK |
           M_ACCOUNTHOOK | M_REPLYHOOK)) {
        /* these hooks allow multiple commands with the same
         * pattern, so if we've already seen this pattern/command pair, just
         * ignore it instead of creating a duplicate */
        if (!str_cmp (ptr->command, command.data)) {
          mem_free (&command.data);
          mem_free (&pattern.data);
          return 0;
        }
      }
      else {
        /* other hooks only allow one command per pattern, so update the
         * entry with the new command.  this currently does not change the
         * order of execution of the hooks, which i think is desirable since
         * a common action to perform is to change the default (.) entry
         * based upon some other information. */
        mem_free (&ptr->command);
        ptr->command = command.data;
        mem_free (&pattern.data);
        return 0;
      }
    }
    if (!ptr->next)
      break;
  }

  if (data &
      (M_SENDHOOK | M_SEND2HOOK | M_SAVEHOOK | M_FCCHOOK | M_MESSAGEHOOK |
       M_REPLYHOOK)) {
    if ((pat =
         mutt_pattern_comp (pattern.data,
                            (data & (M_SENDHOOK | M_SEND2HOOK | M_FCCHOOK)) ?
                            0 : M_FULL_MSG, err)) == NULL)
      goto error;
  }
  else {
    rx = mem_malloc (sizeof (regex_t));
#ifdef M_CRYPTHOOK
    if ((rc =
         REGCOMP (rx, NONULL (pattern.data),
                  ((data & (M_CRYPTHOOK | M_CHARSETHOOK)) ? REG_ICASE : 0)))
        != 0)
#else
    if ((rc =
         REGCOMP (rx, NONULL (pattern.data),
                  (data & (M_CHARSETHOOK | M_ICONVHOOK)) ? REG_ICASE : 0)) !=
        0)
#endif /* M_CRYPTHOOK */
    {
      regerror (rc, rx, err->data, err->dsize);
      regfree (rx);
      mem_free (&rx);
      goto error;
    }
  }

  if (ptr) {
    ptr->next = mem_calloc (1, sizeof (HOOK));
    ptr = ptr->next;
  }
  else
    Hooks = ptr = mem_calloc (1, sizeof (HOOK));
  ptr->type = data;
  ptr->command = command.data;
  ptr->pattern = pat;
  ptr->rx.pattern = pattern.data;
  ptr->rx.rx = rx;
  ptr->rx.not = not;
  return 0;

error:
  mem_free (&pattern.data);
  mem_free (&command.data);
  return (-1);
}

static void delete_hook (HOOK * h)
{
  mem_free (&h->command);
  mem_free (&h->rx.pattern);
  if (h->rx.rx) {
    regfree (h->rx.rx);
  }
  mutt_pattern_free (&h->pattern);
  mem_free (&h);
}

/* Deletes all hooks of type ``type'', or all defined hooks if ``type'' is 0 */
static void delete_hooks (int type)
{
  HOOK *h;
  HOOK *prev;

  while (h = Hooks, h && (type == 0 || type == h->type)) {
    Hooks = h->next;
    delete_hook (h);
  }

  prev = h;                     /* Unused assignment to avoid compiler warnings */

  while (h) {
    if (type == h->type) {
      prev->next = h->next;
      delete_hook (h);
    }
    else
      prev = h;
    h = prev->next;
  }
}

int mutt_parse_unhook (BUFFER * buf, BUFFER * s, unsigned long data,
                       BUFFER * err)
{
  while (MoreArgs (s)) {
    mutt_extract_token (buf, s, 0);
    if (str_cmp ("*", buf->data) == 0) {
      if (current_hook_type) {
        snprintf (err->data, err->dsize,
                  _("unhook: Can't do unhook * from within a hook."));
        return -1;
      }
      delete_hooks (0);
    }
    else {
      int type = mutt_get_hook_type (buf->data);

      if (!type) {
        snprintf (err->data, err->dsize,
                  _("unhook: unknown hook type: %s"), buf->data);
        return (-1);
      }
      if (current_hook_type == type) {
        snprintf (err->data, err->dsize,
                  _("unhook: Can't delete a %s from within a %s."), buf->data,
                  buf->data);
        return -1;
      }
      delete_hooks (type);
    }
  }
  return 0;
}

void mutt_folder_hook (char *path)
{
  HOOK *tmp = Hooks;
  BUFFER err, token;
  char buf[STRING];

  current_hook_type = M_FOLDERHOOK;

  err.data = buf;
  err.dsize = sizeof (buf);
  memset (&token, 0, sizeof (token));
  for (; tmp; tmp = tmp->next) {
    if (!tmp->command)
      continue;

    if (tmp->type & M_FOLDERHOOK) {
      if ((regexec (tmp->rx.rx, path, 0, NULL, 0) == 0) ^ tmp->rx.not) {
        if (mutt_parse_rc_line (tmp->command, &token, &err) == -1) {
          mutt_error ("%s", err.data);
          mutt_sleep (1);       /* pause a moment to let the user see the error */
          if (ERROR_STOP) {
            mem_free (&token.data);
            current_hook_type = 0;
            return;
          }
        }
      }
    }
  }
  mem_free (&token.data);

  current_hook_type = 0;
}

char *mutt_find_hook (int type, const char *pat)
{
  HOOK *tmp = Hooks;

  for (; tmp; tmp = tmp->next)
    if (tmp->type & type) {
      if (regexec (tmp->rx.rx, pat, 0, NULL, 0) == 0)
        return (tmp->command);
    }
  return (NULL);
}

void mutt_message_hook (CONTEXT * ctx, HEADER * hdr, int type)
{
  BUFFER err, token;
  HOOK *hook;
  char buf[STRING];

  current_hook_type = type;

  err.data = buf;
  err.dsize = sizeof (buf);
  memset (&token, 0, sizeof (token));
  for (hook = Hooks; hook; hook = hook->next) {
    if (!hook->command)
      continue;

    if (hook->type & type)
      if ((mutt_pattern_exec (hook->pattern, 0, ctx, hdr) > 0) ^ hook->rx.not)
        if (mutt_parse_rc_line (hook->command, &token, &err) != 0) {
          mutt_error ("%s", err.data);
          mutt_sleep (1);
          if (ERROR_STOP) {
            mem_free (&token.data);
            current_hook_type = 0;
            return;
          }
        }
  }
  mem_free (&token.data);
  current_hook_type = 0;
}

static int
mutt_addr_hook (char *path, size_t pathlen, int type, CONTEXT * ctx,
                HEADER * hdr)
{
  HOOK *hook;

  /* determine if a matching hook exists */
  for (hook = Hooks; hook; hook = hook->next) {
    if (!hook->command)
      continue;

    if (hook->type & type)
      if ((mutt_pattern_exec (hook->pattern, 0, ctx, hdr) > 0) ^ hook->rx.not) {
        mutt_make_string (path, pathlen, hook->command, ctx, hdr);
        return 0;
      }
  }

  return -1;
}

void mutt_default_save (char *path, size_t pathlen, HEADER * hdr)
{
  *path = 0;
  if (mutt_addr_hook (path, pathlen, M_SAVEHOOK, Context, hdr) != 0) {
    char tmp[_POSIX_PATH_MAX];
    ADDRESS *adr;
    ENVELOPE *env = hdr->env;
    int fromMe = mutt_addr_is_user (env->from);

    if (!fromMe && env->reply_to && env->reply_to->mailbox)
      adr = env->reply_to;
    else if (!fromMe && env->from && env->from->mailbox)
      adr = env->from;
    else if (env->to && env->to->mailbox)
      adr = env->to;
    else if (env->cc && env->cc->mailbox)
      adr = env->cc;
    else
      adr = NULL;
    if (adr) {
      mutt_safe_path (tmp, sizeof (tmp), adr);
      snprintf (path, pathlen, "=%s", tmp);
    }
  }
}

void mutt_select_fcc (char *path, size_t pathlen, HEADER * hdr)
{
  ADDRESS *adr;
  char buf[_POSIX_PATH_MAX];
  ENVELOPE *env = hdr->env;

  if (mutt_addr_hook (path, pathlen, M_FCCHOOK, NULL, hdr) != 0) {
    if ((option (OPTSAVENAME) || option (OPTFORCENAME)) &&
        (env->to || env->cc || env->bcc)) {
      adr = env->to ? env->to : (env->cc ? env->cc : env->bcc);
      mutt_safe_path (buf, sizeof (buf), adr);
      mutt_concat_path (path, NONULL (Maildir), buf, pathlen);
      if (!option (OPTFORCENAME) && mx_access (path, W_OK) != 0)
        strfcpy (path, NONULL (Outbox), pathlen);
    }
    else
      strfcpy (path, NONULL (Outbox), pathlen);
  }
  mutt_pretty_mailbox (path);
}

static char *_mutt_string_hook (const char *match, int hook)
{
  HOOK *tmp = Hooks;

  for (; tmp; tmp = tmp->next) {
    if ((tmp->type & hook) && ((match &&
                                regexec (tmp->rx.rx, match, 0, NULL,
                                         0) == 0) ^ tmp->rx.not))
      return (tmp->command);
  }
  return (NULL);
}

char *mutt_charset_hook (const char *chs)
{
  return _mutt_string_hook (chs, M_CHARSETHOOK);
}

char *mutt_iconv_hook (const char *chs)
{
  return _mutt_string_hook (chs, M_ICONVHOOK);
}

char *mutt_crypt_hook (ADDRESS * adr)
{
  return _mutt_string_hook (adr->mailbox, M_CRYPTHOOK);
}

#ifdef USE_SOCKET
void mutt_account_hook (const char *url)
{
  HOOK *hook;
  BUFFER token;
  BUFFER err;
  char buf[STRING];

  err.data = buf;
  err.dsize = sizeof (buf);
  memset (&token, 0, sizeof (token));

  for (hook = Hooks; hook; hook = hook->next) {
    if (!(hook->command && (hook->type & M_ACCOUNTHOOK)))
      continue;

    if ((regexec (hook->rx.rx, url, 0, NULL, 0) == 0) ^ hook->rx.not) {
      if (mutt_parse_rc_line (hook->command, &token, &err) == -1) {
        mutt_error ("%s", err.data);
        mutt_sleep (1);
        if (ERROR_STOP) {
          mem_free (&token.data);
          current_hook_type = 0;
          return;
        }
      }
    }
  }

  mem_free (&token.data);
}
#endif
