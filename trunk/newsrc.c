/*
 * Copyright (C) 1998 Brandon Long <blong@fiction.net>
 * Copyright (C) 1999 Andrej Gritsenko <andrej@lucky.net>
 * Copyright (C) 2000-2001 Vsevolod Volkov <vvv@mutt.org.ua>
 * 
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 * 
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 * 
 *     You should have received a copy of the GNU General Public License
 *     along with this program; if not, write to the Free Software
 *     Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */ 

#include "mutt.h"
#include "mutt_curses.h"
#include "sort.h"
#include "mx.h"
#include "mime.h"
#include "mailbox.h"
#include "nntp.h"
#include "rfc822.h"
#include "rfc1524.h"
#include "rfc2047.h"

#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/stat.h>

void nntp_add_to_list (NNTP_SERVER *s, NNTP_DATA *d)
{
  LIST *l;

  if (!s || !d)
    return;

  l = safe_calloc (1, sizeof (LIST));
  if (s->list)
    s->tail->next = l;
  else
    s->list = l;
  s->tail = l;
  l->data = (void *) d;
}

static int nntp_parse_newsrc_line (NNTP_SERVER *news, char *line)
{
  NNTP_DATA *data;
  char group[LONG_STRING];
  int x = 1;
  char *p = line, *b, *h;
  size_t len;

  while (*p)
  {
    if (*p++ == ',')
      x++;
  }

  p = line;
  while (*p && (*p != ':' && *p != '!')) p++;
  if (!*p)
    return -1;
  len = p + 1 - line;
  if (len > sizeof (group))
    len = sizeof (group);
  strfcpy (group, line, len);
  if ((data = (NNTP_DATA *)hash_find (news->newsgroups, group)) == NULL)
  {
    data = (NNTP_DATA *) safe_calloc (1, sizeof (NNTP_DATA) + strlen (group) + 1);
    data->group = (char *) data + sizeof (NNTP_DATA);
    strcpy (data->group, group);
    data->nserv = news;
    data->deleted = 1;
    if (news->newsgroups->nelem < news->newsgroups->curnelem * 2)
      news->newsgroups = hash_resize (news->newsgroups, news->newsgroups->nelem * 2);
    hash_insert (news->newsgroups, data->group, data, 0);
    nntp_add_to_list (news, data);
  }
  else
    FREE ((void **) &data->entries);

  data->rc = 1;
  data->entries = safe_calloc (x*2, sizeof (NEWSRC_ENTRY));
  data->max = x*2;

  if (*p == ':')
    data->subscribed = 1;
  else
    data->subscribed = 0;

  p++;
  b = p;
  x = 0;
  while (*b)
  {
    while (*p && *p != ',' && *p != '\n') p++;
    if (*p)
    {
      *p = '\0';
      p++;
    }
    if ((h = strchr(b, '-')))
    {
      *h = '\0';
      h++;
      data->entries[x].first = atoi(b);
      data->entries[x].last = atoi(h);
    }
    else
    {
      data->entries[x].first = atoi(b);
      data->entries[x].last = data->entries[x].first;
    }
    b = p;
    if (data->entries[x].last != 0)
      x++;
  }
  if (x && !data->lastMessage)
    data->lastMessage = data->entries[x-1].last;
  data->num = x;
  mutt_newsgroup_stat (data);
  dprint (2, (debugfile, "parse_line: Newsgroup %s\n", data->group));
  
  return 0;
}

static int slurp_newsrc (NNTP_SERVER *news)
{
  FILE *fp;
  char *buf;
  struct stat sb;

  news->stat = stat (news->newsrc, &sb);
  news->size = sb.st_size;
  news->mtime = sb.st_mtime;

  if ((fp = safe_fopen (news->newsrc, "r")) == NULL)
    return -1;
  /* hmm, should we use dotlock? */
  if (mx_lock_file (news->newsrc, fileno (fp), 0, 0, 1))
  {
    fclose (fp);
    return -1;
  }

  buf = safe_malloc (sb.st_size + 1);
  while (fgets (buf, sb.st_size + 1, fp))
    nntp_parse_newsrc_line (news, buf);
  FREE (&buf);

  mx_unlock_file (news->newsrc, fileno (fp), 0);
  fclose (fp);
  return 0;
}

void nntp_cache_expand (char *dst, const char *src)
{
  snprintf (dst, _POSIX_PATH_MAX, "%s/%s", NewsCacheDir, src);
  mutt_expand_path (dst, _POSIX_PATH_MAX);
}

/* Loads $news_cache_dir/.index into memory, loads newsserver data
 * and newsgroup cache names */
static int nntp_parse_cacheindex (NNTP_SERVER *news)
{
  struct stat st;
  char buf[HUGE_STRING], *cp;
  char dir[_POSIX_PATH_MAX], file[_POSIX_PATH_MAX];
  FILE *index;
  NNTP_DATA *data;
  int l, m, t;

  /* check is server name defined or not */
  if (!news || !news->conn || !news->conn->account.host)
    return -1;
  unset_option (OPTNEWSCACHE);
  if (!NewsCacheDir || !*NewsCacheDir)
    return 0;

  strfcpy (dir, NewsCacheDir, sizeof (dir));
  mutt_expand_path (dir, sizeof(dir));

  if (lstat (dir, &st) || (st.st_mode & S_IFDIR) == 0)
  {
    snprintf (buf, sizeof(buf), _("Directory %s not exist. Create it?"), dir);
    if (mutt_yesorno (buf, M_YES) != M_YES || mkdir (dir, (S_IRWXU+S_IRWXG+
	  S_IRWXO)))
    {
      mutt_error _("Cache directory not created!");
      return -1;
    }
    mutt_clear_error();
  }

  set_option (OPTNEWSCACHE);

  FREE (&news->cache);
  snprintf (buf, sizeof(buf), "%s/.index", dir);
  if (!(index = safe_fopen (buf, "a+")))
    return 0;
  rewind (index);
  while (fgets (buf, sizeof(buf), index))
  {
    buf[strlen(buf) - 1] = 0;	/* strip ending '\n' */
    if (!mutt_strncmp (buf, "#: ", 3) &&
	!mutt_strcasecmp (buf+3, news->conn->account.host))
      break;
  }
  while (fgets (buf, sizeof(buf), index))
  {
    cp = buf;
    while (*cp && *cp != ' ') cp++;
    if (!*cp) continue;
    cp[0] = 0;
    if (!mutt_strcmp (buf, "#:"))
      break;
    sscanf (cp + 1, "%s %d %d", file, &l, &m);
    if (!mutt_strcmp (buf, "ALL"))
    {
      news->cache = safe_strdup (file);
      news->newgroups_time = m;
    }
    else if (news->newsgroups)
    {
      if ((data = (NNTP_DATA *)hash_find (news->newsgroups, buf)) == NULL)
      {
	data = (NNTP_DATA *) safe_calloc (1, sizeof (NNTP_DATA) + strlen (buf) + 1);
	data->group = (char *) data + sizeof (NNTP_DATA);
	strcpy(data->group, buf);
	data->nserv = news;
	data->deleted = 1;
	if (news->newsgroups->nelem < news->newsgroups->curnelem * 2)
	  news->newsgroups = hash_resize (news->newsgroups, news->newsgroups->nelem * 2);
	hash_insert (news->newsgroups, data->group, data, 0);
	nntp_add_to_list (news, data);
      }
      data->cache = safe_strdup (file);
      t = 0;
      if (!data->firstMessage || data->lastMessage < m)
	t = 1;
      if (!data->firstMessage)
	data->firstMessage = l;
      if (data->lastMessage < m)
	data->lastMessage = m;
      data->lastCached = m;
      if (t || !data->unread)
	mutt_newsgroup_stat (data);
    }
  }
  fclose (index);
  return 0;
}

const char *
nntp_format_str (char *dest, size_t destlen, char op, const char *src,
		const char *fmt, const char *ifstring, const char *elsestring,
		unsigned long data, format_flag flags)
{
  char fn[SHORT_STRING], tmp[SHORT_STRING];

  switch (op)
  {
    case 's':
      strncpy (fn, NewsServer, sizeof(fn) - 1);
      mutt_strlower (fn);
      snprintf (tmp, sizeof (tmp), "%%%ss", fmt);
      snprintf (dest, destlen, tmp, fn);
      break;
  }
  return (src);
}

/* nntp_parse_url: given an NNPT URL, return host, port,
 * username, password and newsgroup will recognise. */
int nntp_parse_url (const char *server, ACCOUNT *acct,
		    char *group, size_t group_len)
{
  ciss_url_t url;
  char *c;
  int ret = -1;

  /* Defaults */
  acct->flags = 0;
  acct->port = NNTP_PORT;
  acct->type = M_ACCT_TYPE_NNTP;

  c = safe_strdup (server);
  url_parse_ciss (&url, c);

  if (url.scheme == U_NNTP || url.scheme == U_NNTPS)
  {
    if (url.scheme == U_NNTPS)
    {
      acct->flags |= M_ACCT_SSL;
      acct->port = NNTP_SSL_PORT;
    }

    *group = '\0';
    if (url.path)
      strfcpy (group, url.path, group_len);

    ret = mutt_account_fromurl (acct, &url);
  }

  FREE (&c);
  return ret;
}

void nntp_expand_path (char *line, size_t len, ACCOUNT *acct)
{
  ciss_url_t url;

  url.path = safe_strdup (line);
  mutt_account_tourl (acct, &url);
  url_ciss_tostring (&url, line, len, 0);
  FREE (&url.path);
}

/*
 * Automatically loads a newsrc into memory, if necessary.
 * Checks the size/mtime of a newsrc file, if it doesn't match, load
 * again.  Hmm, if a system has broken mtimes, this might mean the file
 * is reloaded every time, which we'd have to fix.
 *
 * a newsrc file is a line per newsgroup, with the newsgroup, then a 
 * ':' denoting subscribed or '!' denoting unsubscribed, then a 
 * comma separated list of article numbers and ranges.
 */
NNTP_SERVER *mutt_select_newsserver (char *server)
{
  char file[_POSIX_PATH_MAX];
  char *buf, *p;
  LIST *list;
  ACCOUNT acct;
  NNTP_SERVER *serv;
  CONNECTION *conn;

  if (!server || !*server)
  {
    mutt_error _("No newsserver defined!");
    return NULL;
  }

  buf = p = safe_calloc (strlen (server) + 10, sizeof (char));
  if (url_check_scheme (server) == U_UNKNOWN)
  {
    strcpy (buf, "nntp://");
    p = strchr (buf, '\0');
  }
  strcpy (p, server);

  if ((nntp_parse_url (buf, &acct, file, sizeof (file))) < 0 || *file)
  {
    FREE (&buf);
    mutt_error (_("%s is an invalid newsserver specification!"), server);
    return NULL;
  }
  FREE (&buf);

  conn = mutt_conn_find (NULL, &acct);
  if (!conn)
    return NULL;

  mutt_FormatString (file, sizeof (file), NONULL (NewsRc), nntp_format_str, 0, 0);
  mutt_expand_path (file, sizeof (file));

  serv = (NNTP_SERVER *)conn->data;
  if (serv)
  {
    struct stat sb;

    /* externally modified? */
    if (serv->stat != stat (file, &sb) || (!serv->stat &&
       (serv->size != sb.st_size || serv->mtime != sb.st_mtime)))
    {
      for (list = serv->list; list; list = list->next)
      {
	NNTP_DATA *data = (NNTP_DATA *) list->data;

	if (data)
	{
	  data->subscribed = 0;
	  data->rc = 0;
	  data->num = 0;
	}
      }
      slurp_newsrc (serv);
      nntp_clear_cacheindex (serv);
    }

    if (serv->status == NNTP_BYE)
      serv->status = NNTP_NONE;
    nntp_check_newgroups (serv, 0);
    return serv;
  }

  /* New newsserver */
  serv = safe_calloc (1, sizeof (NNTP_SERVER));
  serv->conn = conn;
  serv->newsrc = safe_strdup (file);
  serv->newsgroups = hash_create (1009);
  slurp_newsrc (serv);			/* load .newsrc */
  nntp_parse_cacheindex (serv);		/* load .index */
  if (option (OPTNEWSCACHE) && serv->cache && nntp_get_cache_all (serv) >= 0)
    nntp_check_newgroups (serv, 1);
  else if (nntp_get_active (serv) < 0)
  {
    hash_destroy (&serv->newsgroups, nntp_delete_data);
    for (list = serv->list; list; list = list->next)
      list->data = NULL;
    mutt_free_list (&serv->list);
    FREE (&serv->newsrc);
    FREE (&serv->cache);
    FREE (&serv);
    return NULL;
  }
  nntp_clear_cacheindex (serv);
  conn->data = (void *)serv;

  return serv;
}

/* 
 * full status flags are not supported by nntp, but we can fake some
 * of them.  This is how:
 * Read = a read message number is in the .newsrc
 * New = a message is new since we last read this newsgroup
 * Old = anything else
 * So, Read is marked as such in the newsrc, old is anything that is 
 * "skipped" in the newsrc, and new is anything not in the newsrc nor
 * in the cache. By skipped, I mean before the last unread message
 */
void nntp_get_status (CONTEXT *ctx, HEADER *h, char *group, int article)
{
  NNTP_DATA *data = (NNTP_DATA *) ctx->data;
  int x;

  if (group)
    data = (NNTP_DATA *) hash_find (data->nserv->newsgroups, group);

  if (!data)
  {
#ifdef DEBUG
    if (group)
      dprint (3, (debugfile, "newsgroup %s not found\n", group));
#endif
    return;
  }

  for (x = 0; x < data->num; x++)
  {
    if ((article >= data->entries[x].first) &&
	(article <= data->entries[x].last))
    {
      /* we cannot use mutt_set_flag() because mx_update_context()
	 didn't called yet */
      h->read = 1;
      return;
    }
  }
  /* If article was not cached yet, it is new! :) */
  if (!data->cache || article > data->lastCached)
    return;
  /* Old articles are articles which aren't read but an article after them
   * has been cached */
  if (option (OPTMARKOLD))
    h->old = 1;
}

void mutt_newsgroup_stat (NNTP_DATA *data)
{
  int i;
  unsigned int first, last;

  data->unread = 0;
  if (data->lastMessage == 0 || data->firstMessage > data->lastMessage)
    return;

  data->unread = data->lastMessage - data->firstMessage + 1;
  for (i = 0; i < data->num; i++)
  {
    first = data->entries[i].first;
    if (first < data->firstMessage)
      first = data->firstMessage;
    last = data->entries[i].last;
    if (last > data->lastMessage)
      last = data->lastMessage;
    if (first <= last)
      data->unread -= last - first + 1;
  }
}

static int puti (char *line, int num)
{
  char *p, s[32];

  for (p = s; num; )
  {
    *p++ = '0' + num % 10;
    num /= 10;
  }
  while (p > s)
    *line++ = *--p, num++;
  *line = '\0';
  return num;
}

static void nntp_create_newsrc_line (NNTP_DATA *data, char **buf, char **pline, size_t *buflen)
{
  char *line = *pline;
  size_t len = *buflen - (*pline - *buf);
  int x, i;

  if (len < LONG_STRING * 10)
  {
    len += *buflen;
    *buflen *= 2;
    line = *buf;
    safe_realloc (buf, *buflen);
    line = *buf + (*pline - line);
  }
  strcpy (line, data->group);
  len -= strlen (line) + 1;
  line += strlen (line);
  *line++ = data->subscribed ? ':' : '!';
  *line++ = ' ';
  *line = '\0';

  for (x = 0; x < data->num; x++)
  {
    if (len < LONG_STRING)
    {
      len += *buflen;
      *buflen *= 2;
      *pline = line;
      line = *buf;
      safe_realloc (buf, *buflen);
      line = *buf + (*pline - line);
    }
    if (x)
    {
      *line++ = ',';
      len--;
    }

#if 0
    if (data->entries[x].first == data->entries[x].last)
      snprintf (line, len, "%d%n", data->entries[x].first, &i);
    else
      snprintf (line, len, "%d-%d%n", 
	    data->entries[x].first, data->entries[x].last, &i);
    len -= i;
    line += i;
#else
    i = puti (line, data->entries[x].first);
    line +=i; len -= i;
    if (data->entries[x].first != data->entries[x].last)
    {
      *line++ = '-';
      len--;
      i = puti (line, data->entries[x].last);
      line +=i; len -= i;
    }
#endif
  }
  *line++ = '\n';
  *line = '\0';
  *pline = line;
}

void newsrc_gen_entries (CONTEXT *ctx)
{
  NNTP_DATA *data = (NNTP_DATA *)ctx->data;
  int series, x;
  unsigned int last = 0, first = 1;
  int save_sort = SORT_ORDER;

  if (Sort != SORT_ORDER)
  {
    save_sort = Sort;
    Sort = SORT_ORDER;
    mutt_sort_headers (ctx, 0);
  }

  if (!data->max)
  {
    data->entries = safe_calloc (5, sizeof (NEWSRC_ENTRY));
    data->max = 5;
  }

  /*
   * Set up to fake initial sequence from 1 to the article before the 
   * first article in our list
   */
  data->num = 0;
  series = 1;

  for (x = 0; x < ctx->msgcount; x++)
  {
    if (series) /* search for first unread */
    {
      /*
       * We don't actually check sequential order, since we mark 
       * "missing" entries as read/deleted
       */
      last = ctx->hdrs[x]->article_num;
      if (last >= data->firstMessage && !ctx->hdrs[x]->deleted &&
	    !ctx->hdrs[x]->read)
      {
	if (data->num >= data->max)
	{
	  data->max = data->max * 2;
	  safe_realloc (&data->entries, 
	      data->max * sizeof (NEWSRC_ENTRY));
	}
	data->entries[data->num].first = first;
	data->entries[data->num].last = last - 1;
	data->num++;
	series = 0;
      }
    }
    else /* search for first read */
    {
      if (ctx->hdrs[x]->deleted || ctx->hdrs[x]->read)
      {
	first = last + 1;
	series = 1;
      }
      last = ctx->hdrs[x]->article_num;
    }
  }
  if (series && first <= data->lastLoaded)
  {
    if (data->num >= data->max)
    {
      data->max = data->max * 2;
      safe_realloc (&data->entries, 
		    data->max * sizeof (NEWSRC_ENTRY));
    }
    data->entries[data->num].first = first;
    data->entries[data->num].last = data->lastLoaded;
    data->num++;
  }

  if (save_sort != Sort)
  {
    Sort = save_sort;
    mutt_sort_headers (ctx, 0);
  }
}

int mutt_newsrc_update (NNTP_SERVER *news)
{
  char *buf, *line;
  NNTP_DATA *data;
  LIST *tmp;
  int r = -1;
  size_t len, llen;

  if (!news)
    return -1;
  llen = len = 10 * LONG_STRING;
  line = buf = safe_calloc (1, len);
  /* we will generate full newsrc here */
  for (tmp = news->list; tmp; tmp = tmp->next)
  {
    data = (NNTP_DATA *) tmp->data;
    if (!data || !data->rc)
      continue;
    nntp_create_newsrc_line (data, &buf, &line, &llen);
    dprint (2, (debugfile, "Added to newsrc: %s", line));
    line += strlen (line);
  }
  /* newrc being fully rewritten */
  if (news->newsrc &&
     (r = mutt_update_list_file (news->newsrc, NULL, "", buf)) == 0)
  {
    struct stat st;

    stat (news->newsrc, &st);
    news->size = st.st_size;
    news->mtime = st.st_mtime;
  }
  FREE (&buf);
  return r;
}

static FILE *mutt_mkname (char *s)
{
  char buf[_POSIX_PATH_MAX], *pc;
  int fd;
  FILE *fp;

  nntp_cache_expand (buf, s);
  if ((fp = safe_fopen (buf, "w")))
    return fp;

  nntp_cache_expand (buf, "cache-XXXXXX");
  pc = buf + strlen (buf) - 12;	/* positioning to "cache-XXXXXX" */
  if ((fd = mkstemp (buf)) == -1)
    return NULL;
  strcpy (s, pc);	/* generated name */
  return fdopen (fd, "w");
}

/* Updates info into .index file: ALL or about selected newsgroup */
static int nntp_update_cacheindex (NNTP_SERVER *serv, NNTP_DATA *data)
{
  char buf[LONG_STRING], *key = "ALL";
  char file[_POSIX_PATH_MAX];

  if (!serv || !serv->conn || !serv->conn->account.host)
    return -1;

  if (data && data->group)
  {
    key = data->group;
    snprintf (buf, sizeof (buf), "%s %s %d %d", key, data->cache,
	  data->firstMessage, data->lastLoaded);
  }
  else
  {
    strfcpy (file, serv->cache, sizeof (file));
    snprintf (buf, sizeof (buf), "ALL %s 0 %d", file, (int)serv->newgroups_time);
  }
  nntp_cache_expand (file, ".index");
  return mutt_update_list_file (file, serv->conn->account.host, key, buf);
}

/* Remove cache files of unsubscribed newsgroups */
void nntp_clear_cacheindex (NNTP_SERVER *news)
{
  NNTP_DATA *data;
  LIST *tmp;

  if (option (OPTSAVEUNSUB) || !news)
    return;

  for (tmp = news->list; tmp; tmp = tmp->next)
  {
    data = (NNTP_DATA *) tmp->data;
    if (!data || data->subscribed || !data->cache)
      continue;
    nntp_delete_cache (data);
    dprint (2, (debugfile, "Removed from .index: %s\n", data->group));
  }
  return;
}

int nntp_save_cache_index (NNTP_SERVER *news)
{
  char buf[HUGE_STRING];
  char file[_POSIX_PATH_MAX];
  NNTP_DATA *d;
  FILE *f;
  LIST *l;

  if (!news || !news->newsgroups)
    return -1;
  if (!option (OPTNEWSCACHE))
    return 0;

  if (news->cache)
  {
    nntp_cache_expand (file, news->cache);
    unlink (file);
    f = safe_fopen (file, "w");
  }
  else
  {
    strfcpy (buf, news->conn->account.host, sizeof(buf));
    f = mutt_mkname (buf);
    news->cache = safe_strdup (buf);
    nntp_cache_expand (file, buf);
  }
  if (!f)
    return -1;

  for (l = news->list; l; l = l->next)
  {
    if ((d = (NNTP_DATA *)l->data) && !d->deleted)
    {
      if (d->desc)
	snprintf (buf, sizeof(buf), "%s %d %d %c %s\n", d->group,
	      d->lastMessage, d->firstMessage, d->allowed ? 'y' : 'n',
 	      d->desc);
      else
	snprintf (buf, sizeof(buf), "%s %d %d %c\n", d->group,
	      d->lastMessage, d->firstMessage, d->allowed ? 'y' : 'n');
      if (fputs (buf, f) == EOF)
      {
	fclose (f);
	unlink (file);
	return -1;
      }
    }
  }
  fclose (f);

  if (nntp_update_cacheindex (news, NULL))
  {
    unlink (file);
    return -1;
  }
  return 0;
}

int nntp_save_cache_group (CONTEXT *ctx)
{
  char buf[HUGE_STRING], addr[STRING];
  char file[_POSIX_PATH_MAX];
  FILE *f;
  HEADER *h;
  struct tm *tm;
  int i = 0, save = SORT_ORDER;
  int prev = 0;

  if (!option (OPTNEWSCACHE))
    return 0;
  if (!ctx || !ctx->data || ctx->magic != M_NNTP)
    return -1;

  if (((NNTP_DATA *)ctx->data)->cache)
  {
    nntp_cache_expand (file, ((NNTP_DATA *)ctx->data)->cache);
    unlink (file);
    f = safe_fopen (file, "w");
  }
  else
  {
    snprintf (buf, sizeof(buf), "%s-%s",
	((NNTP_DATA *)ctx->data)->nserv->conn->account.host,
	((NNTP_DATA *)ctx->data)->group);
    f = mutt_mkname (buf);
    ((NNTP_DATA *)ctx->data)->cache = safe_strdup (buf);
    nntp_cache_expand (file, buf);
  }
  if (!f)
    return -1;

  if (Sort != SORT_ORDER)
  {
    save = Sort;
    Sort = SORT_ORDER;
    mutt_sort_headers (ctx, 0);
  }

  /* Save only $nntp_context messages... */
  ((NNTP_DATA *)ctx->data)->lastCached = 0;
  if (NntpContext && ctx->msgcount > NntpContext)
    i = ctx->msgcount - NntpContext;
  for (; i < ctx->msgcount; i++)
  {
    if (!ctx->hdrs[i]->deleted && ctx->hdrs[i]->article_num != prev)
    {
      h = ctx->hdrs[i];
      addr[0] = 0;
      rfc822_write_address (addr, sizeof(addr), h->env->from, 0);
      tm = gmtime (&h->date_sent);
      snprintf (buf, sizeof(buf),
	    "%d\t%s\t%s\t%d %s %d %02d:%02d:%02d GMT\t%s\t",
	    h->article_num, h->env->subject, addr, tm->tm_mday,
	    Months[tm->tm_mon], tm->tm_year+1900, tm->tm_hour, tm->tm_min,
	    tm->tm_sec, h->env->message_id);
      fputs (buf, f);
      if (h->env->references)
	mutt_write_references (h->env->references, f);
      snprintf (buf, sizeof(buf), "\t%ld\t%d\tXref: %s\n", h->content->length,
	    h->lines, NONULL(h->env->xref));
      if (fputs (buf, f) == EOF)
      {
	fclose (f);
	unlink (file);
	return -1;
      }
    }
    prev = ctx->hdrs[i]->article_num;
  }

  if (save != Sort)
  {
    Sort = save;
    mutt_sort_headers (ctx, 0);
  }
  fclose (f);

  if (nntp_update_cacheindex (((NNTP_DATA *)ctx->data)->nserv,
	(NNTP_DATA *)ctx->data))
  {
    unlink (file);
    return -1;
  }
  ((NNTP_DATA *)ctx->data)->lastCached = ((NNTP_DATA *)ctx->data)->lastLoaded;
  return 0;
}

void nntp_delete_cache (NNTP_DATA *data)
{
  char buf[_POSIX_PATH_MAX];

  if (!option (OPTNEWSCACHE) || !data || !data->cache || !data->nserv)
    return;

  nntp_cache_expand (buf, data->cache);
  unlink (buf);
  FREE (&data->cache);
  data->lastCached = 0;
  nntp_cache_expand (buf, ".index");
  mutt_update_list_file (buf, data->nserv->conn->account.host, data->group, NULL);
}

NNTP_DATA *mutt_newsgroup_subscribe (NNTP_SERVER *news, char *group)
{
  NNTP_DATA *data;

  if (!news || !news->newsgroups || !group || !*group)
    return NULL;
  if (!(data = (NNTP_DATA *)hash_find (news->newsgroups, group)))
  {
    data = (NNTP_DATA *) safe_calloc (1, sizeof (NNTP_DATA) + strlen (group) + 1);
    data->group = (char *) data + sizeof (NNTP_DATA);
    strcpy (data->group, group);
    data->nserv = news;
    data->deleted = 1;
    if (news->newsgroups->nelem < news->newsgroups->curnelem * 2)
      news->newsgroups = hash_resize (news->newsgroups, news->newsgroups->nelem * 2);
    hash_insert (news->newsgroups, data->group, data, 0);
    nntp_add_to_list (news, data);
  }
  if (!data->subscribed)
  {
    data->subscribed = 1;
    data->rc = 1;
  }
  return data;
}

NNTP_DATA *mutt_newsgroup_unsubscribe (NNTP_SERVER *news, char *group)
{
  NNTP_DATA *data;

  if (!news || !news->newsgroups || !group || !*group ||
	!(data = (NNTP_DATA *)hash_find (news->newsgroups, group)))
    return NULL;
  if (data->subscribed)
  {
    data->subscribed = 0;
    if (!option (OPTSAVEUNSUB))
      data->rc = 0;
  }
  return data;
}

NNTP_DATA *mutt_newsgroup_catchup (NNTP_SERVER *news, char *group)
{
  NNTP_DATA *data;

  if (!news || !news->newsgroups || !group || !*group ||
      !(data = (NNTP_DATA *)hash_find (news->newsgroups, group)))
    return NULL;
  if (!data->max)
  {
    data->entries = safe_calloc (5, sizeof (NEWSRC_ENTRY));
    data->max = 5;
  }
  data->num = 1;
  data->entries[0].first = 1;
  data->unread = 0;
  data->entries[0].last = data->lastMessage;
  if (Context && Context->data == data)
  {
    int x;

    for (x = 0; x < Context->msgcount; x++)
      mutt_set_flag (Context, Context->hdrs[x], M_READ, 1);
  }
  return data;
}

NNTP_DATA *mutt_newsgroup_uncatchup (NNTP_SERVER *news, char *group)
{
  NNTP_DATA *data;

  if (!news || !news->newsgroups || !group || !*group ||
      !(data = (NNTP_DATA *)hash_find (news->newsgroups, group)))
    return NULL;
  if (!data->max)
  {
    data->entries = safe_calloc (5, sizeof (NEWSRC_ENTRY));
    data->max = 5;
  }
  data->num = 1;
  data->entries[0].first = 1;
  data->entries[0].last = data->firstMessage - 1;
  if (Context && Context->data == data)
  {
    int x;

    data->unread = Context->msgcount;
    for (x = 0; x < Context->msgcount; x++)
      mutt_set_flag (Context, Context->hdrs[x], M_READ, 0);
  }
  else
    data->unread = data->lastMessage - data->entries[0].last;
  return data;
}

/* this routine gives the first newsgroup with new messages */
void nntp_buffy (char *s)
{
  LIST *list;

  for (list = CurrentNewsSrv->list; list; list = list->next)
  {
    NNTP_DATA *data = (NNTP_DATA *) list->data;

    if (data && data->subscribed && data->unread)
    {
      if (Context && Context->magic == M_NNTP &&
	  !mutt_strcmp (data->group, ((NNTP_DATA *) Context->data)->group))
      {
	unsigned int i, unread = 0;

	for (i = 0; i < Context->msgcount; i++)
	  if (!Context->hdrs[i]->read && !Context->hdrs[i]->deleted)
	    unread++;
	if (!unread)
	  continue;
      }
      strcpy (s, data->group);
      break;
    }
  }
}
