/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 * Copyright (C) 2004 g10 Code GmbH
 *
 * Parts were written/modified by:
 * Nico Golde <nico@ngolde.de>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "mutt.h"
#include "mutt_curses.h"
#include "mutt_idna.h"
#include "mutt_menu.h"
#include "rfc1524.h"
#include "mime.h"
#include "attach.h"
#include "mapping.h"
#include "sort.h"
#include "charset.h"
#include "mx.h"
#include "buffy.h"
#include "compose.h"

#ifdef MIXMASTER
#include "remailer.h"
#endif

#ifdef USE_NNTP
#include "nntp.h"
#endif

#include "lib/mem.h"
#include "lib/intl.h"
#include "lib/str.h"

#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

static const char *There_are_no_attachments = N_("There are no attachments.");

#define CHECK_COUNT if (idxlen == 0) { mutt_error _(There_are_no_attachments); break; }



enum {
  HDR_FROM = 1,
  HDR_TO,
  HDR_CC,
  HDR_BCC,
  HDR_SUBJECT,
  HDR_REPLYTO,
  HDR_FCC,

#ifdef MIXMASTER
  HDR_MIX,
#endif

  HDR_CRYPT,
  HDR_CRYPTINFO,

#ifdef USE_NNTP
  HDR_NEWSGROUPS,
  HDR_FOLLOWUPTO,
  HDR_XCOMMENTTO,
#endif

#ifndef USE_NNTP
  HDR_ATTACH = (HDR_FCC + 5)    /* where to start printing the attachments */
#else
  HDR_ATTACH = (HDR_FCC + 7)
#endif
};

#define HDR_XOFFSET 14
#define TITLE_FMT "%14s"        /* Used for Prompts, which are ASCII */
#define W (COLS - HDR_XOFFSET - SidebarWidth)

static char *Prompts[] = {
  "From: ",
  "To: ",
  "Cc: ",
  "Bcc: ",
  "Subject: ",
  "Reply-To: ",
  "Fcc: "
#ifdef USE_NNTP
#ifdef MIXMASTER
    , ""
#endif
    , "", "", "Newsgroups: ", "Followup-To: ", "X-Comment-To: "
#endif
};

static struct mapping_t ComposeHelp[] = {
  {N_("Send"), OP_COMPOSE_SEND_MESSAGE},
  {N_("Abort"), OP_EXIT},
  {"To", OP_COMPOSE_EDIT_TO},
  {"CC", OP_COMPOSE_EDIT_CC},
  {"Subj", OP_COMPOSE_EDIT_SUBJECT},
  {N_("Attach file"), OP_COMPOSE_ATTACH_FILE},
  {N_("Descrip"), OP_COMPOSE_EDIT_DESCRIPTION},
  {N_("Help"), OP_HELP},
  {NULL}
};

#ifdef USE_NNTP
static struct mapping_t ComposeNewsHelp[] = {
  {N_("Send"), OP_COMPOSE_SEND_MESSAGE},
  {N_("Abort"), OP_EXIT},
  {"Newsgroups", OP_COMPOSE_EDIT_NEWSGROUPS},
  {"Subj", OP_COMPOSE_EDIT_SUBJECT},
  {N_("Attach file"), OP_COMPOSE_ATTACH_FILE},
  {N_("Descrip"), OP_COMPOSE_EDIT_DESCRIPTION},
  {N_("Help"), OP_HELP},
  {NULL}
};
#endif

static void snd_entry (char *b, size_t blen, MUTTMENU * menu, int num)
{
  mutt_FormatString (b, blen, NONULL (AttachFormat), mutt_attach_fmt,
                     (unsigned long) (((ATTACHPTR **) menu->data)[num]),
                     M_FORMAT_STAT_FILE | M_FORMAT_ARROWCURSOR);
}



#include "mutt_crypt.h"

static void redraw_crypt_lines (HEADER * msg)
{
  int off = 0;

  if ((WithCrypto & APPLICATION_PGP) && (WithCrypto & APPLICATION_SMIME)) {
    if (!msg->security)
      mvaddstr (HDR_CRYPT, SidebarWidth, "    Security: ");
    else if (msg->security & APPLICATION_SMIME)
      mvaddstr (HDR_CRYPT, SidebarWidth, "      S/MIME: ");
    else if (msg->security & APPLICATION_PGP)
      mvaddstr (HDR_CRYPT, SidebarWidth, "         PGP: ");
  }
  else if ((WithCrypto & APPLICATION_SMIME))
    mvaddstr (HDR_CRYPT, SidebarWidth, "      S/MIME: ");
  else if ((WithCrypto & APPLICATION_PGP))
    mvaddstr (HDR_CRYPT, SidebarWidth, "         PGP: ");
  else
    return;

  if ((msg->security & (ENCRYPT | SIGN)) == (ENCRYPT | SIGN))
    addstr (_("Sign, Encrypt"));
  else if (msg->security & ENCRYPT)
    addstr (_("Encrypt"));
  else if (msg->security & SIGN)
    addstr (_("Sign"));
  else
    addstr (_("Clear"));

  if ((WithCrypto & APPLICATION_PGP))
    if ((msg->security & APPLICATION_PGP)
        && (msg->security & (ENCRYPT | SIGN))) {
      if ((msg->security & INLINE))
        addstr (_(" (inline)"));
      else
        addstr (_(" (PGP/MIME)"));
    }
  clrtoeol ();

  move (HDR_CRYPTINFO, SidebarWidth);
  clrtoeol ();
  if ((WithCrypto & APPLICATION_PGP)
      && msg->security & APPLICATION_PGP && msg->security & SIGN)
    printw ("%s%s", _("     sign as: "),
            PgpSignAs ? PgpSignAs : _("<default>"));

  if ((WithCrypto & APPLICATION_SMIME)
      && msg->security & APPLICATION_SMIME && msg->security & SIGN) {
    printw ("%s%s", _("     sign as: "),
            SmimeDefaultKey ? SmimeDefaultKey : _("<default>"));
  }

  if ((WithCrypto & APPLICATION_SMIME)
      && (msg->security & APPLICATION_SMIME)
      && (msg->security & ENCRYPT)
      && SmimeCryptAlg && *SmimeCryptAlg) {
    mvprintw (HDR_CRYPTINFO, SidebarWidth + 40, "%s%s", _("Encrypt with: "),
              NONULL (SmimeCryptAlg));
    off = 20;
  }
}


#ifdef MIXMASTER

static void redraw_mix_line (LIST * chain)
{
  int c;
  char *t;

  mvaddstr (HDR_MIX, SidebarWidth, "         Mix: ");

  if (!chain) {
    addstr ("<no chain defined>");
    clrtoeol ();
    return;
  }

  for (c = 12; chain; chain = chain->next) {
    t = chain->data;
    if (t && t[0] == '0' && t[1] == '\0')
      t = "<random>";

    if (c + safe_strlen (t) + 2 >= COLS - SidebarWidth)
      break;

    addstr (NONULL (t));
    if (chain->next)
      addstr (", ");

    c += safe_strlen (t) + 2;
  }
}
#endif /* MIXMASTER */

static int check_attachments (ATTACHPTR ** idx, short idxlen)
{
  int i, r;
  struct stat st;
  char pretty[_POSIX_PATH_MAX], msg[_POSIX_PATH_MAX + SHORT_STRING];

  for (i = 0; i < idxlen; i++) {
    strfcpy (pretty, idx[i]->content->filename, sizeof (pretty));
    if (stat (idx[i]->content->filename, &st) != 0) {
      mutt_pretty_mailbox (pretty);
      mutt_error (_("%s [#%d] no longer exists!"), pretty, i + 1);
      return -1;
    }

    if (idx[i]->content->stamp < st.st_mtime) {
      mutt_pretty_mailbox (pretty);
      snprintf (msg, sizeof (msg), _("%s [#%d] modified. Update encoding?"),
                pretty, i + 1);

      if ((r = mutt_yesorno (msg, M_YES)) == M_YES)
        mutt_update_encoding (idx[i]->content);
      else if (r == -1)
        return -1;
    }
  }

  return 0;
}

static void draw_envelope_addr (int line, ADDRESS * addr)
{
  char buf[STRING];

  buf[0] = 0;
  rfc822_write_address (buf, sizeof (buf), addr, 1);
  mvprintw (line, SidebarWidth, TITLE_FMT, Prompts[line - 1]);
  mutt_paddstr (W, buf);
}

static void draw_envelope (HEADER * msg, char *fcc)
{
  draw_envelope_addr (HDR_FROM, msg->env->from);
#ifdef USE_NNTP
  if (!option (OPTNEWSSEND)) {
#endif
    draw_envelope_addr (HDR_TO, msg->env->to);
    draw_envelope_addr (HDR_CC, msg->env->cc);
    draw_envelope_addr (HDR_BCC, msg->env->bcc);
#ifdef USE_NNTP
  }
  else {
    mvprintw (HDR_TO, SidebarWidth, TITLE_FMT, Prompts[HDR_NEWSGROUPS - 1]);
    mutt_paddstr (W, NONULL (msg->env->newsgroups));
    mvprintw (HDR_CC, SidebarWidth, TITLE_FMT, Prompts[HDR_FOLLOWUPTO - 1]);
    mutt_paddstr (W, NONULL (msg->env->followup_to));
    if (option (OPTXCOMMENTTO)) {
      mvprintw (HDR_BCC, 0, TITLE_FMT, Prompts[HDR_XCOMMENTTO - 1]);
      mutt_paddstr (W, NONULL (msg->env->x_comment_to));
    }
  }
#endif
  mvprintw (HDR_SUBJECT, SidebarWidth, TITLE_FMT, Prompts[HDR_SUBJECT - 1]);
  mutt_paddstr (W, NONULL (msg->env->subject));
  draw_envelope_addr (HDR_REPLYTO, msg->env->reply_to);
  mvprintw (HDR_FCC, SidebarWidth, TITLE_FMT, Prompts[HDR_FCC - 1]);
  mutt_paddstr (W, fcc);

  if (WithCrypto)
    redraw_crypt_lines (msg);

#ifdef MIXMASTER
  redraw_mix_line (msg->chain);
#endif

  SETCOLOR (MT_COLOR_STATUS);
  mvaddstr (HDR_ATTACH - 1, SidebarWidth, _("-- Attachments"));
  BKGDSET (MT_COLOR_STATUS);
  clrtoeol ();

  BKGDSET (MT_COLOR_NORMAL);
  SETCOLOR (MT_COLOR_NORMAL);
}

static int edit_address_list (int line, ADDRESS ** addr)
{
  char buf[HUGE_STRING] = "";   /* needs to be large for alias expansion */
  char *err = NULL;

  mutt_addrlist_to_local (*addr);
  rfc822_write_address (buf, sizeof (buf), *addr, 0);
  if (mutt_get_field (Prompts[line - 1], buf, sizeof (buf), M_ALIAS) == 0) {
    rfc822_free_address (addr);
    *addr = mutt_parse_adrlist (*addr, buf);
    *addr = mutt_expand_aliases (*addr);
  }

  if (option (OPTNEEDREDRAW)) {
    unset_option (OPTNEEDREDRAW);
    return (REDRAW_FULL);
  }

  if (mutt_addrlist_to_idna (*addr, &err) != 0) {
    mutt_error (_("Warning: '%s' is a bad IDN."), err);
    mutt_refresh ();
    FREE (&err);
  }

  /* redraw the expanded list so the user can see the result */
  buf[0] = 0;
  rfc822_write_address (buf, sizeof (buf), *addr, 1);
  move (line, HDR_XOFFSET + SidebarWidth);
  mutt_paddstr (W, buf);

  return 0;
}

static int delete_attachment (MUTTMENU * menu, short *idxlen, int x)
{
  ATTACHPTR **idx = (ATTACHPTR **) menu->data;
  int y;

  menu->redraw = REDRAW_INDEX | REDRAW_STATUS;

  if (x == 0 && menu->max == 1) {
    mutt_error _("You may not delete the only attachment.");

    idx[x]->content->tagged = 0;
    return (-1);
  }

  for (y = 0; y < *idxlen; y++) {
    if (idx[y]->content->next == idx[x]->content) {
      idx[y]->content->next = idx[x]->content->next;
      break;
    }
  }

  idx[x]->content->next = NULL;
  idx[x]->content->parts = NULL;
  mutt_free_body (&(idx[x]->content));
  FREE (&idx[x]->tree);
  FREE (&idx[x]);
  for (; x < *idxlen - 1; x++)
    idx[x] = idx[x + 1];
  menu->max = --(*idxlen);

  return (0);
}

static void update_idx (MUTTMENU * menu, ATTACHPTR ** idx, short idxlen)
{
  idx[idxlen]->level = (idxlen > 0) ? idx[idxlen - 1]->level : 0;
  if (idxlen)
    idx[idxlen - 1]->content->next = idx[idxlen]->content;
  idx[idxlen]->content->aptr = idx[idxlen];
  menu->current = idxlen++;
  mutt_update_tree (idx, idxlen);
  menu->max = idxlen;
  return;
}


/* 
 * cum_attachs_size: Cumulative Attachments Size
 *
 * Returns the total number of bytes used by the attachments in the
 * attachment list _after_ content-transfer-encodings have been
 * applied.
 * 
 */

static unsigned long cum_attachs_size (MUTTMENU * menu)
{
  size_t s;
  unsigned short i;
  ATTACHPTR **idx = menu->data;
  CONTENT *info;
  BODY *b;

  for (i = 0, s = 0; i < menu->max; i++) {
    b = idx[i]->content;

    if (!b->content)
      b->content = mutt_get_content_info (b->filename, b);

    if ((info = b->content)) {
      switch (b->encoding) {
      case ENCQUOTEDPRINTABLE:
        s += 3 * (info->lobin + info->hibin) + info->ascii + info->crlf;
        break;
      case ENCBASE64:
        s += (4 * (info->lobin + info->hibin + info->ascii + info->crlf)) / 3;
        break;
      default:
        s += info->lobin + info->hibin + info->ascii + info->crlf;
        break;
      }
    }
  }

  return s;
}

/*
 * compose_format_str()
 *
 * %a = total number of attachments 
 * %h = hostname  [option]
 * %l = approx. length of current message (in bytes) 
 * %v = Mutt version 
 *
 * This function is similar to status_format_str().  Look at that function for
 * help when modifying this function.
 */

static const char *compose_format_str (char *buf, size_t buflen, char op,
                                       const char *src, const char *prefix,
                                       const char *ifstring,
                                       const char *elsestring,
                                       unsigned long data, format_flag flags)
{
  char fmt[SHORT_STRING], tmp[SHORT_STRING];
  int optional = (flags & M_FORMAT_OPTIONAL);
  MUTTMENU *menu = (MUTTMENU *) data;

  *buf = 0;
  switch (op) {
  case 'a':                    /* total number of attachments */
    snprintf (fmt, sizeof (fmt), "%%%sd", prefix);
    snprintf (buf, buflen, fmt, menu->max);
    break;

  case 'h':                    /* hostname */
    snprintf (fmt, sizeof (fmt), "%%%ss", prefix);
    snprintf (buf, buflen, fmt, NONULL (Hostname));
    break;

  case 'l':                    /* approx length of current message in bytes */
    snprintf (fmt, sizeof (fmt), "%%%ss", prefix);
    mutt_pretty_size (tmp, sizeof (tmp), menu ? cum_attachs_size (menu) : 0);
    snprintf (buf, buflen, fmt, tmp);
    break;

  case 'v':
    snprintf (fmt, sizeof (fmt), "Mutt-ng %%s");
    snprintf (buf, buflen, fmt, MUTT_VERSION);
    break;

  case 0:
    *buf = 0;
    return (src);

  default:
    snprintf (buf, buflen, "%%%s%c", prefix, op);
    break;
  }

  if (optional)
    compose_status_line (buf, buflen, menu, ifstring);
  else if (flags & M_FORMAT_OPTIONAL)
    compose_status_line (buf, buflen, menu, elsestring);

  return (src);
}

static void compose_status_line (char *buf, size_t buflen, MUTTMENU * menu,
                                 const char *p)
{
  mutt_FormatString (buf, buflen, p, compose_format_str,
                     (unsigned long) menu, 0);
}


/* return values:
 *
 * 1	message should be postponed
 * 0	normal exit
 * -1	abort message
 */
int mutt_compose_menu (HEADER * msg,    /* structure for new message */
                       char *fcc,       /* where to save a copy of the message */
                       size_t fcclen, HEADER * cur)
{                               /* current message */
  char helpstr[SHORT_STRING];
  char buf[LONG_STRING];
  char fname[_POSIX_PATH_MAX];
  MUTTMENU *menu;
  ATTACHPTR **idx = NULL;
  short idxlen = 0;
  short idxmax = 0;
  int i, close = 0;
  int r = -1;                   /* return value */
  int op = 0;
  int loop = 1;
  int fccSet = 0;               /* has the user edited the Fcc: field ? */
  CONTEXT *ctx = NULL, *this = NULL;

  /* Sort, SortAux could be changed in mutt_index_menu() */
  int oldSort, oldSortAux;
  struct stat st;

#ifdef USE_NNTP
  int news = 0;                 /* is it a news article ? */

  if (option (OPTNEWSSEND))
    news++;
#endif

  mutt_attach_init (msg->content);
  idx = mutt_gen_attach_list (msg->content, -1, idx, &idxlen, &idxmax, 0, 1);

  menu = mutt_new_menu ();
  menu->menu = MENU_COMPOSE;
  menu->offset = HDR_ATTACH;
  menu->max = idxlen;
  menu->make_entry = snd_entry;
  menu->tag = mutt_tag_attach;
  menu->data = idx;
#ifdef USE_NNTP
  if (news)
    menu->help =
      mutt_compile_help (helpstr, sizeof (helpstr), MENU_COMPOSE,
                         ComposeNewsHelp);
  else
#endif
    menu->help =
      mutt_compile_help (helpstr, sizeof (helpstr), MENU_COMPOSE,
                         ComposeHelp);

  if (option (OPTMBOXPANE))
    buffy_check (1);
  while (loop) {
#ifdef USE_NNTP
    unset_option (OPTNEWS);     /* for any case */
#endif
    switch (op = mutt_menuLoop (menu)) {
    case OP_REDRAW:
      draw_envelope (msg, fcc);
      menu->offset = HDR_ATTACH;
      menu->pagelen = LINES - HDR_ATTACH - 2;
      break;
    case OP_COMPOSE_EDIT_FROM:
      menu->redraw = edit_address_list (HDR_FROM, &msg->env->from);
      mutt_message_hook (NULL, msg, M_SEND2HOOK);
      break;
    case OP_COMPOSE_EDIT_TO:
#ifdef USE_NNTP
      if (!news) {
#endif
        menu->redraw = edit_address_list (HDR_TO, &msg->env->to);
        mutt_message_hook (NULL, msg, M_SEND2HOOK);
#ifdef USE_NNTP
      }
#endif
      break;
    case OP_COMPOSE_EDIT_BCC:
#ifdef USE_NNTP
      if (!news) {
#endif
        menu->redraw = edit_address_list (HDR_BCC, &msg->env->bcc);
        mutt_message_hook (NULL, msg, M_SEND2HOOK);
#ifdef USE_NNTP
      }
#endif
      break;
    case OP_COMPOSE_EDIT_CC:
#ifdef USE_NNTP
      if (!news) {
#endif
        menu->redraw = edit_address_list (HDR_CC, &msg->env->cc);
        mutt_message_hook (NULL, msg, M_SEND2HOOK);
#ifdef USE_NNTP
      }
#endif
      break;
#ifdef USE_NNTP
    case OP_COMPOSE_EDIT_NEWSGROUPS:
      if (news) {
        if (msg->env->newsgroups)
          strfcpy (buf, msg->env->newsgroups, sizeof (buf));
        else
          buf[0] = 0;
        if (mutt_get_field ("Newsgroups: ", buf, sizeof (buf), 0) == 0
            && buf[0]) {
          FREE (&msg->env->newsgroups);
          str_skip_trailws (buf);
          msg->env->newsgroups = safe_strdup (str_skip_initws (buf));
          move (HDR_TO, HDR_XOFFSET);
          clrtoeol ();
          if (msg->env->newsgroups)
            printw ("%-*.*s", W, W, msg->env->newsgroups);
        }
      }
      break;

    case OP_COMPOSE_EDIT_FOLLOWUP_TO:
      if (news) {
        buf[0] = 0;
        if (msg->env->followup_to)
          strfcpy (buf, msg->env->followup_to, sizeof (buf));
        if (mutt_get_field ("Followup-To: ", buf, sizeof (buf), 0) == 0
            && buf[0]) {
          FREE (&msg->env->followup_to);
          str_skip_trailws (buf);
          msg->env->followup_to = safe_strdup (str_skip_initws (buf));
          move (HDR_CC, HDR_XOFFSET);
          clrtoeol ();
          if (msg->env->followup_to)
            printw ("%-*.*s", W, W, msg->env->followup_to);
        }
      }
      break;

    case OP_COMPOSE_EDIT_X_COMMENT_TO:
      if (news && option (OPTXCOMMENTTO)) {
        buf[0] = 0;
        if (msg->env->x_comment_to)
          strfcpy (buf, msg->env->x_comment_to, sizeof (buf));
        if (mutt_get_field ("X-Comment-To: ", buf, sizeof (buf), 0) == 0
            && buf[0]) {
          FREE (&msg->env->x_comment_to);
          msg->env->x_comment_to = safe_strdup (buf);
          move (HDR_BCC, HDR_XOFFSET);
          clrtoeol ();
          if (msg->env->x_comment_to)
            printw ("%-*.*s", W, W, msg->env->x_comment_to);
        }
      }
      break;
#endif
    case OP_COMPOSE_EDIT_SUBJECT:
      if (msg->env->subject)
        strfcpy (buf, msg->env->subject, sizeof (buf));
      else
        buf[0] = 0;
      if (mutt_get_field ("Subject: ", buf, sizeof (buf), 0) == 0) {
        str_replace (&msg->env->subject, buf);
        move (HDR_SUBJECT, HDR_XOFFSET + SidebarWidth);
        clrtoeol ();
        if (msg->env->subject)
          mutt_paddstr (W, msg->env->subject);
      }
      mutt_message_hook (NULL, msg, M_SEND2HOOK);
      break;
    case OP_COMPOSE_EDIT_REPLY_TO:
      menu->redraw = edit_address_list (HDR_REPLYTO, &msg->env->reply_to);
      mutt_message_hook (NULL, msg, M_SEND2HOOK);
      break;
    case OP_COMPOSE_EDIT_FCC:
      strfcpy (buf, fcc, sizeof (buf));
      if (mutt_get_field ("Fcc: ", buf, sizeof (buf), M_FILE | M_CLEAR) == 0) {
        strfcpy (fcc, buf, _POSIX_PATH_MAX);
        mutt_pretty_mailbox (fcc);
        move (HDR_FCC, HDR_XOFFSET + SidebarWidth);
        mutt_paddstr (W, fcc);
        fccSet = 1;
      }
      MAYBE_REDRAW (menu->redraw);
      mutt_message_hook (NULL, msg, M_SEND2HOOK);
      break;
    case OP_COMPOSE_EDIT_MESSAGE:
      if (Editor && (safe_strcmp ("builtin", Editor) != 0)
          && !option (OPTEDITHDRS)) {
        mutt_edit_file (Editor, msg->content->filename);
        mutt_update_encoding (msg->content);
        menu->redraw = REDRAW_CURRENT | REDRAW_STATUS;
        mutt_message_hook (NULL, msg, M_SEND2HOOK);
        break;
      }
      /* fall through */
    case OP_COMPOSE_EDIT_HEADERS:
      if (safe_strcmp ("builtin", Editor) != 0 &&
          (op == OP_COMPOSE_EDIT_HEADERS ||
           (op == OP_COMPOSE_EDIT_MESSAGE && option (OPTEDITHDRS)))) {
        char *tag = NULL, *err = NULL;

        mutt_env_to_local (msg->env);
        mutt_edit_headers (NONULL (Editor), msg->content->filename, msg,
                           fcc, fcclen);
        if (mutt_env_to_idna (msg->env, &tag, &err)) {
          mutt_error (_("Bad IDN in \"%s\": '%s'"), tag, err);
          FREE (&err);
        }
      }
      else {
        /* this is grouped with OP_COMPOSE_EDIT_HEADERS because the
           attachment list could change if the user invokes ~v to edit
           the message with headers, in which we need to execute the
           code below to regenerate the index array */
        mutt_builtin_editor (msg->content->filename, msg, cur);
      }
      mutt_update_encoding (msg->content);

      /* attachments may have been added */
      if (idxlen && idx[idxlen - 1]->content->next) {
        for (i = 0; i < idxlen; i++)
          FREE (&idx[i]);
        idxlen = 0;
        idx =
          mutt_gen_attach_list (msg->content, -1, idx, &idxlen, &idxmax, 0,
                                1);
        menu->data = idx;
        menu->max = idxlen;
      }

      menu->redraw = REDRAW_FULL;
      mutt_message_hook (NULL, msg, M_SEND2HOOK);
      break;



    case OP_COMPOSE_ATTACH_KEY:
      if (!(WithCrypto & APPLICATION_PGP))
        break;
      if (idxlen == idxmax) {
        safe_realloc (&idx, sizeof (ATTACHPTR *) * (idxmax += 5));
        menu->data = idx;
      }

      idx[idxlen] = (ATTACHPTR *) safe_calloc (1, sizeof (ATTACHPTR));
      if ((idx[idxlen]->content =
           crypt_pgp_make_key_attachment (NULL)) != NULL) {
        update_idx (menu, idx, idxlen++);
        menu->redraw |= REDRAW_INDEX;
      }
      else
        FREE (&idx[idxlen]);

      menu->redraw |= REDRAW_STATUS;

      if (option (OPTNEEDREDRAW)) {
        menu->redraw = REDRAW_FULL;
        unset_option (OPTNEEDREDRAW);
      }

      mutt_message_hook (NULL, msg, M_SEND2HOOK);
      break;


    case OP_COMPOSE_ATTACH_FILE:
      {
        char *prompt, **files;
        int error, numfiles;

        fname[0] = 0;
        prompt = _("Attach file");
        numfiles = 0;
        files = NULL;

        if (_mutt_enter_fname
            (prompt, fname, sizeof (fname), &menu->redraw, 0, 1, &files,
             &numfiles) == -1 || *fname == '\0')
          break;

        if (idxlen + numfiles >= idxmax) {
          safe_realloc (&idx,
                        sizeof (ATTACHPTR *) * (idxmax += 5 + numfiles));
          menu->data = idx;
        }

        error = 0;
        if (numfiles > 1)
          mutt_message _("Attaching selected files...");

        for (i = 0; i < numfiles; i++) {
          char *att = files[i];

          idx[idxlen] = (ATTACHPTR *) safe_calloc (1, sizeof (ATTACHPTR));
          idx[idxlen]->content = mutt_make_file_attach (att);
          if (idx[idxlen]->content != NULL)
            update_idx (menu, idx, idxlen++);
          else {
            error = 1;
            mutt_error (_("Unable to attach %s!"), att);
            FREE (&idx[idxlen]);
          }
        }

        FREE (&files);
        if (!error)
          mutt_clear_error ();

        menu->redraw |= REDRAW_INDEX | REDRAW_STATUS;
      }
      mutt_message_hook (NULL, msg, M_SEND2HOOK);
      break;

    case OP_COMPOSE_ATTACH_MESSAGE:
#ifdef USE_NNTP
    case OP_COMPOSE_ATTACH_NEWS_MESSAGE:
#endif
      {
        char *prompt;
        HEADER *h;

        fname[0] = 0;
        prompt = _("Open mailbox to attach message from");

#ifdef USE_NNTP
        unset_option (OPTNEWS);
        if (op == OP_COMPOSE_ATTACH_NEWS_MESSAGE) {
          if (!(CurrentNewsSrv = mutt_select_newsserver (NewsServer)))
            break;

          prompt = _("Open newsgroup to attach message from");
          set_option (OPTNEWS);
        }
#endif

        if (Context)
#ifdef USE_NNTP
          if ((op == OP_COMPOSE_ATTACH_MESSAGE) ^ (Context->magic == M_NNTP))
#endif
          {
            strfcpy (fname, NONULL (Context->path), sizeof (fname));
            mutt_pretty_mailbox (fname);
          }

        if (mutt_enter_fname (prompt, fname, sizeof (fname), &menu->redraw, 1)
            == -1 || !fname[0])
          break;

#ifdef USE_NNTP
        if (option (OPTNEWS))
          nntp_expand_path (fname, sizeof (fname),
                            &CurrentNewsSrv->conn->account);
        else
#endif
          mutt_expand_path (fname, sizeof (fname));
#ifdef USE_IMAP
        if (mx_get_magic (fname) != M_IMAP)
#endif
#ifdef USE_POP
          if (mx_get_magic (fname) != M_POP)
#endif
#ifdef USE_NNTP
            if (mx_get_magic (fname) != M_NNTP && !option (OPTNEWS))
#endif
              /* check to make sure the file exists and is readable */
              if (access (fname, R_OK) == -1) {
                mutt_perror (fname);
                break;
              }

        menu->redraw = REDRAW_FULL;

        ctx = mx_open_mailbox (fname, M_READONLY, NULL);
        if (ctx == NULL) {
          mutt_perror (fname);
          break;
        }

        if (!ctx->msgcount) {
          mx_close_mailbox (ctx, NULL);
          FREE (&ctx);
          mutt_error _("No messages in that folder.");

          break;
        }

        this = Context;         /* remember current folder and sort methods */
        oldSort = Sort;
        oldSortAux = SortAux;

        Context = ctx;
        set_option (OPTATTACHMSG);
        mutt_message _("Tag the messages you want to attach!");

        close = mutt_index_menu ();
        unset_option (OPTATTACHMSG);

        if (!Context) {
          /* go back to the folder we started from */
          Context = this;
          /* Restore old $sort and $sort_aux */
          Sort = oldSort;
          SortAux = oldSortAux;
          menu->redraw |= REDRAW_INDEX | REDRAW_STATUS;
          break;
        }

        if (idxlen + Context->tagged >= idxmax) {
          safe_realloc (&idx,
                        sizeof (ATTACHPTR *) * (idxmax +=
                                                5 + Context->tagged));
          menu->data = idx;
        }

        for (i = 0; i < Context->msgcount; i++) {
          h = Context->hdrs[i];
          if (h->tagged) {
            idx[idxlen] = (ATTACHPTR *) safe_calloc (1, sizeof (ATTACHPTR));
            idx[idxlen]->content = mutt_make_message_attach (Context, h, 1);
            if (idx[idxlen]->content != NULL)
              update_idx (menu, idx, idxlen++);
            else {
              mutt_error _("Unable to attach!");

              FREE (&idx[idxlen]);
            }
          }
        }
        menu->redraw |= REDRAW_FULL;

        if (close == OP_QUIT)
          mx_close_mailbox (Context, NULL);
        else
          mx_fastclose_mailbox (Context);
        FREE (&Context);

        /* go back to the folder we started from */
        Context = this;
        /* Restore old $sort and $sort_aux */
        Sort = oldSort;
        SortAux = oldSortAux;
      }
      mutt_message_hook (NULL, msg, M_SEND2HOOK);
      break;

    case OP_DELETE:
      CHECK_COUNT;
      if (delete_attachment (menu, &idxlen, menu->current) == -1)
        break;
      mutt_update_tree (idx, idxlen);
      if (idxlen) {
        if (menu->current > idxlen - 1)
          menu->current = idxlen - 1;
      }
      else
        menu->current = 0;

      if (menu->current == 0)
        msg->content = idx[0]->content;

      menu->redraw |= REDRAW_STATUS;
      mutt_message_hook (NULL, msg, M_SEND2HOOK);
      break;

#define CURRENT idx[menu->current]->content

    case OP_COMPOSE_TOGGLE_RECODE:
      {
        CHECK_COUNT;
        if (!mutt_is_text_part (CURRENT)) {
          mutt_error (_("Recoding only affects text attachments."));
          break;
        }
        CURRENT->noconv = !CURRENT->noconv;
        if (CURRENT->noconv)
          mutt_message (_("The current attachment won't be converted."));
        else
          mutt_message (_("The current attachment will be converted."));
        menu->redraw = REDRAW_CURRENT;
        mutt_message_hook (NULL, msg, M_SEND2HOOK);
        break;
      }
#undef CURRENT

    case OP_COMPOSE_EDIT_DESCRIPTION:
      CHECK_COUNT;
      strfcpy (buf,
               idx[menu->current]->content->description ?
               idx[menu->current]->content->description : "", sizeof (buf));
      /* header names should not be translated */
      if (mutt_get_field ("Description: ", buf, sizeof (buf), 0) == 0) {
        str_replace (&idx[menu->current]->content->description, buf);
        menu->redraw = REDRAW_CURRENT;
      }
      mutt_message_hook (NULL, msg, M_SEND2HOOK);
      break;

    case OP_COMPOSE_UPDATE_ENCODING:
      CHECK_COUNT;
      if (menu->tagprefix) {
        BODY *top;

        for (top = msg->content; top; top = top->next) {
          if (top->tagged)
            mutt_update_encoding (top);
        }
        menu->redraw = REDRAW_FULL;
      }
      else {
        mutt_update_encoding (idx[menu->current]->content);
        menu->redraw = REDRAW_CURRENT | REDRAW_STATUS;
      }
      mutt_message_hook (NULL, msg, M_SEND2HOOK);
      break;

    case OP_COMPOSE_TOGGLE_DISPOSITION:
      /* toggle the content-disposition between inline/attachment */
      idx[menu->current]->content->disposition =
        (idx[menu->current]->content->disposition ==
         DISPINLINE) ? DISPATTACH : DISPINLINE;
      menu->redraw = REDRAW_CURRENT;
      break;

    case OP_EDIT_TYPE:
      CHECK_COUNT;
      {
        mutt_edit_content_type (NULL, idx[menu->current]->content, NULL);

        /* this may have been a change to text/something */
        mutt_update_encoding (idx[menu->current]->content);

        menu->redraw = REDRAW_CURRENT;
      }
      mutt_message_hook (NULL, msg, M_SEND2HOOK);
      break;

    case OP_COMPOSE_EDIT_ENCODING:
      CHECK_COUNT;
      strfcpy (buf, ENCODING (idx[menu->current]->content->encoding),
               sizeof (buf));
      if (mutt_get_field ("Content-Transfer-Encoding: ", buf,
                          sizeof (buf), 0) == 0 && buf[0]) {
        if ((i = mutt_check_encoding (buf)) != ENCOTHER && i != ENCUUENCODED) {
          idx[menu->current]->content->encoding = i;
          menu->redraw = REDRAW_CURRENT | REDRAW_STATUS;
          mutt_clear_error ();
        }
        else
          mutt_error _("Invalid encoding.");
      }
      mutt_message_hook (NULL, msg, M_SEND2HOOK);
      break;

    case OP_COMPOSE_SEND_MESSAGE:

      /* Note: We don't invoke send2-hook here, since we want to leave
       * users an opportunity to change settings from the ":" prompt.
       */

      if (check_attachments (idx, idxlen) != 0) {
        menu->redraw = REDRAW_FULL;
        break;
      }


#ifdef MIXMASTER
      if (msg->chain && mix_check_message (msg) != 0)
        break;
#endif

      if (!fccSet && *fcc) {
        if ((i = query_quadoption (OPT_COPY,
                                   _("Save a copy of this message?"))) == -1)
          break;
        else if (i == M_NO)
          *fcc = 0;
      }

      loop = 0;
      r = 0;
      break;

    case OP_COMPOSE_EDIT_FILE:
      CHECK_COUNT;
      mutt_edit_file (NONULL (Editor), idx[menu->current]->content->filename);
      mutt_update_encoding (idx[menu->current]->content);
      menu->redraw = REDRAW_CURRENT | REDRAW_STATUS;
      mutt_message_hook (NULL, msg, M_SEND2HOOK);
      break;

    case OP_COMPOSE_TOGGLE_UNLINK:
      CHECK_COUNT;
      idx[menu->current]->content->unlink =
        !idx[menu->current]->content->unlink;

#if 0
      /* OPTRESOLVE is otherwise ignored on this menu.
       * Where's the bug?
       */

      if (option (OPTRESOLVE) && menu->current + 1 < menu->max)
        menu->current++;
# endif
      menu->redraw = REDRAW_INDEX;
      /* No send2hook since this doesn't change the message. */
      break;

    case OP_COMPOSE_GET_ATTACHMENT:
      CHECK_COUNT;
      if (menu->tagprefix) {
        BODY *top;

        for (top = msg->content; top; top = top->next) {
          if (top->tagged)
            mutt_get_tmp_attachment (top);
        }
        menu->redraw = REDRAW_FULL;
      }
      else if (mutt_get_tmp_attachment (idx[menu->current]->content) == 0)
        menu->redraw = REDRAW_CURRENT;

      /* No send2hook since this doesn't change the message. */
      break;

    case OP_COMPOSE_RENAME_FILE:
      CHECK_COUNT;
      strfcpy (fname, idx[menu->current]->content->filename, sizeof (fname));
      mutt_pretty_mailbox (fname);
      if (mutt_get_field (_("Rename to: "), fname, sizeof (fname), M_FILE)
          == 0 && fname[0]) {
        if (stat (idx[menu->current]->content->filename, &st) == -1) {
          mutt_error (_("Can't stat %s: %s"), fname, strerror (errno));
          break;
        }

        mutt_expand_path (fname, sizeof (fname));
        if (mutt_rename_file (idx[menu->current]->content->filename, fname))
          break;

        str_replace (&idx[menu->current]->content->filename, fname);
        menu->redraw = REDRAW_CURRENT;

        if (idx[menu->current]->content->stamp >= st.st_mtime)
          mutt_stamp_attachment (idx[menu->current]->content);

      }
      mutt_message_hook (NULL, msg, M_SEND2HOOK);
      break;

    case OP_COMPOSE_NEW_MIME:
      {
        char type[STRING];
        char *p;
        int itype;
        FILE *fp;

        CLEARLINE (LINES - 1);
        fname[0] = 0;
        if (mutt_get_field (_("New file: "), fname, sizeof (fname), M_FILE)
            != 0 || !fname[0])
          continue;
        mutt_expand_path (fname, sizeof (fname));

        /* Call to lookup_mime_type () ?  maybe later */
        type[0] = 0;
        if (mutt_get_field ("Content-Type: ", type, sizeof (type), 0) != 0
            || !type[0])
          continue;

        if (!(p = strchr (type, '/'))) {
          mutt_error _("Content-Type is of the form base/sub");

          continue;
        }
        *p++ = 0;
        if ((itype = mutt_check_mime_type (type)) == TYPEOTHER) {
          mutt_error (_("Unknown Content-Type %s"), type);
          continue;
        }
        if (idxlen == idxmax) {
          safe_realloc (&idx, sizeof (ATTACHPTR *) * (idxmax += 5));
          menu->data = idx;
        }

        idx[idxlen] = (ATTACHPTR *) safe_calloc (1, sizeof (ATTACHPTR));
        /* Touch the file */
        if (!(fp = safe_fopen (fname, "w"))) {
          mutt_error (_("Can't create file %s"), fname);
          FREE (&idx[idxlen]);
          continue;
        }
        fclose (fp);

        if ((idx[idxlen]->content = mutt_make_file_attach (fname)) == NULL) {
          mutt_error
            _("What we have here is a failure to make an attachment");
          continue;
        }
        update_idx (menu, idx, idxlen++);

        idx[menu->current]->content->type = itype;
        str_replace (&idx[menu->current]->content->subtype, p);
        idx[menu->current]->content->unlink = 1;
        menu->redraw |= REDRAW_INDEX | REDRAW_STATUS;

        if (mutt_compose_attachment (idx[menu->current]->content)) {
          mutt_update_encoding (idx[menu->current]->content);
          menu->redraw = REDRAW_FULL;
        }
      }
      mutt_message_hook (NULL, msg, M_SEND2HOOK);
      break;

    case OP_COMPOSE_EDIT_MIME:
      CHECK_COUNT;
      if (mutt_edit_attachment (idx[menu->current]->content)) {
        mutt_update_encoding (idx[menu->current]->content);
        menu->redraw = REDRAW_FULL;
      }
      mutt_message_hook (NULL, msg, M_SEND2HOOK);
      break;

    case OP_VIEW_ATTACH:
    case OP_DISPLAY_HEADERS:
      CHECK_COUNT;
      mutt_attach_display_loop (menu, op, NULL, NULL, NULL, &idx, &idxlen,
                                NULL, 0);
      menu->redraw = REDRAW_FULL;
      /* no send2hook, since this doesn't modify the message */
      break;

    case OP_SAVE:
      CHECK_COUNT;
      mutt_save_attachment_list (NULL, menu->tagprefix,
                                 menu->tagprefix ? msg->content : idx[menu->
                                                                      current]->
                                 content, NULL, menu);
      MAYBE_REDRAW (menu->redraw);
      /* no send2hook, since this doesn't modify the message */
      break;

    case OP_PRINT:
      CHECK_COUNT;
      mutt_print_attachment_list (NULL, menu->tagprefix,
                                  menu->tagprefix ? msg->content : idx[menu->
                                                                       current]->
                                  content);
      /* no send2hook, since this doesn't modify the message */
      break;

    case OP_PIPE:
    case OP_FILTER:
      CHECK_COUNT;
      mutt_pipe_attachment_list (NULL, menu->tagprefix,
                                 menu->tagprefix ? msg->content : idx[menu->
                                                                      current]->
                                 content, op == OP_FILTER);
      if (op == OP_FILTER)      /* cte might have changed */
        menu->redraw = menu->tagprefix ? REDRAW_FULL : REDRAW_CURRENT;
      menu->redraw |= REDRAW_STATUS;
      mutt_message_hook (NULL, msg, M_SEND2HOOK);
      break;

    case OP_EXIT:
      if ((i =
           query_quadoption (OPT_POSTPONE,
                             _("Postpone this message?"))) == M_NO) {
        while (idxlen-- > 0) {
          /* avoid freeing other attachments */
          idx[idxlen]->content->next = NULL;
          idx[idxlen]->content->parts = NULL;
          mutt_free_body (&idx[idxlen]->content);
          FREE (&idx[idxlen]->tree);
          FREE (&idx[idxlen]);
        }
        FREE (&idx);
        idxlen = 0;
        idxmax = 0;
        r = -1;
        loop = 0;
        break;
      }
      else if (i == -1)
        break;                  /* abort */

      /* fall through to postpone! */

    case OP_COMPOSE_POSTPONE_MESSAGE:

      if (check_attachments (idx, idxlen) != 0) {
        menu->redraw = REDRAW_FULL;
        break;
      }

      loop = 0;
      r = 1;
      break;

    case OP_COMPOSE_ISPELL:
      endwin ();
      snprintf (buf, sizeof (buf), "%s -x %s", NONULL (Ispell),
                msg->content->filename);
      if (mutt_system (buf) == -1)
        mutt_error (_("Error running \"%s\"!"), buf);
      else {
        mutt_update_encoding (msg->content);
        menu->redraw |= REDRAW_STATUS;
      }
      break;

    case OP_COMPOSE_WRITE_MESSAGE:

      fname[0] = '\0';
      if (Context) {
        strfcpy (fname, NONULL (Context->path), sizeof (fname));
        mutt_pretty_mailbox (fname);
      }
      if (idxlen)
        msg->content = idx[0]->content;
      if (mutt_enter_fname
          (_("Write message to mailbox"), fname, sizeof (fname),
           &menu->redraw, 1) != -1 && fname[0]) {
        mutt_message (_("Writing message to %s ..."), fname);
        mutt_expand_path (fname, sizeof (fname));

        if (msg->content->next)
          msg->content = mutt_make_multipart (msg->content);

        if (mutt_write_fcc (NONULL (fname), msg, NULL, 1, NULL) < 0)
          msg->content = mutt_remove_multipart (msg->content);
        else
          mutt_message _("Message written.");
      }
      break;



    case OP_COMPOSE_PGP_MENU:
      if (!(WithCrypto & APPLICATION_PGP))
        break;
      if ((WithCrypto & APPLICATION_SMIME)
          && msg->security & APPLICATION_SMIME) {
        if (mutt_yesorno (_("S/MIME already selected. Clear & continue ? "),
                          M_YES) != M_YES) {
          mutt_clear_error ();
          break;
        }
        msg->security = 0;
      }
      msg->security = crypt_pgp_send_menu (msg, &menu->redraw);
      redraw_crypt_lines (msg);
      mutt_message_hook (NULL, msg, M_SEND2HOOK);
      break;


    case OP_FORGET_PASSPHRASE:
      crypt_forget_passphrase ();
      break;


    case OP_COMPOSE_SMIME_MENU:
      if (!(WithCrypto & APPLICATION_SMIME))
        break;

      if ((WithCrypto & APPLICATION_PGP)
          && msg->security & APPLICATION_PGP) {
        if (mutt_yesorno (_("PGP already selected. Clear & continue ? "),
                          M_YES) != M_YES) {
          mutt_clear_error ();
          break;
        }
        msg->security = 0;
      }
      msg->security = crypt_smime_send_menu (msg, &menu->redraw);
      redraw_crypt_lines (msg);
      mutt_message_hook (NULL, msg, M_SEND2HOOK);
      break;


#ifdef MIXMASTER
    case OP_COMPOSE_MIX:

      mix_make_chain (&msg->chain, &menu->redraw);
      mutt_message_hook (NULL, msg, M_SEND2HOOK);
      break;
#endif

    }

    /* Draw formated compose status line */
    if (menu->redraw & REDRAW_STATUS) {
      compose_status_line (buf, sizeof (buf), menu, NONULL (ComposeFormat));
      CLEARLINE (option (OPTSTATUSONTOP) ? 0 : LINES - 2);
      SETCOLOR (MT_COLOR_STATUS);
      printw ("%-*.*s", COLS, COLS, buf);
      SETCOLOR (MT_COLOR_NORMAL);
      menu->redraw &= ~REDRAW_STATUS;
    }
  }

  mutt_menuDestroy (&menu);

  if (idxlen) {
    msg->content = idx[0]->content;
    for (i = 0; i < idxlen; i++) {
      idx[i]->content->aptr = NULL;
      FREE (&idx[i]);
    }
  }
  else
    msg->content = NULL;

  FREE (&idx);

  return (r);
}
