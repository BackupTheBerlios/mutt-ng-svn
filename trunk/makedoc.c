/*
 * Copyright (C) 1999-2000 Thomas Roessler <roessler@does-not-exist.org>
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
 *     Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.
 */ 

/**
 ** This program parses mutt's init.h and generates documentation in
 ** three different formats:
 **
 ** -> a commented muttrc configuration file
 ** -> nroff, suitable for inclusion in a manual page
 ** -> linuxdoc-sgml, suitable for inclusion in the 
 **    SGML-based manual
 **
 **/

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <errno.h>

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif

#ifndef HAVE_STRERROR
#ifndef STDC_HEADERS
extern int sys_nerr;
extern char *sys_errlist[];
#endif

#define strerror(x) ((x) > 0 && (x) < sys_nerr) ? sys_errlist[(x)] : 0
#endif /* !HAVE_STRERROR */

extern int optind;

#define BUFFSIZE 2048

#define STRLEN(s) (s ? strlen(s) : 0)

typedef struct {
  short seen;
  char* name;
  char* descr;
} var_t;

static int outcount = 0;
static var_t* outbuf = NULL;

static int var_cmp (const void* a, const void* b) {
  return (strcmp (((var_t*) a)->name, ((var_t*) b)->name));
}

enum output_formats_t
{
  F_CONF, F_MAN, F_SGML, F_NONE
};

#define D_NL            (1 << 0)
#define D_EM            (1 << 1)
#define D_BF            (1 << 2)
#define D_TAB           (1 << 3)
#define D_NP            (1 << 4)
#define D_INIT          (1 << 5)
#define D_DL            (1 << 6)
#define D_DT            (1 << 7)

enum
{
  SP_START_EM,
  SP_START_BF,
  SP_END_FT,
  SP_NEWLINE,
  SP_NEWPAR,
  SP_STR,
  SP_START_TAB,
  SP_END_TAB,
  SP_START_DL,
  SP_DT,
  SP_DD,
  SP_END_DL,
  SP_REFER
};

enum output_formats_t OutputFormat = F_NONE;
char *Progname;
short Debug = 0;

static char *get_token (char *, size_t, char *);
static char *skip_ws (char *);
static const char *type2human (int);
static int buff2type (const char *);
static int flush_doc (int);
static int handle_docline (char *, int);
static int print_it (int, char *, int);
static void print_confline (const char *, int, const char *);
static void handle_confline (char *);
static void makedoc (FILE *, FILE *);
static void pretty_default (char *, size_t, const char *, int);
static int sgml_fputc (int);
static int sgml_fputs (const char *);
static void add_var (const char*);
static int add_s (const char*);
static int add_c (int);

int main (int argc, char *argv[])
{
  int c;
  FILE *f;

  if ((Progname = strrchr (argv[0], '/')))
    Progname++;
  else
    Progname = argv[0];
  
  while ((c = getopt (argc, argv, "cmsd")) != EOF)
  {
    switch (c)
    {
      case 'c': OutputFormat = F_CONF; break;
      case 'm': OutputFormat = F_MAN; break;
      case 's': OutputFormat = F_SGML; break;
      case 'd': Debug++; break;
      default: 
      {
        fprintf (stderr, "%s: bad command line parameter.\n", Progname);
        exit (1);
      }
    }
  }

  if (optind != argc)
  {
    if ((f = fopen (argv[optind], "r")) == NULL)
    {
      fprintf (stderr, "%s: Can't open %s (%s).\n",
               Progname, argv[optind], strerror (errno));
      exit (1);
    }
  }
  else 
    f = stdin;

  switch (OutputFormat)
  {
    case F_CONF: 
    case F_MAN:  
    case F_SGML: makedoc (f, stdout); break;
    default:
    {
      fprintf (stderr, "%s: No output format specified.\n",
               Progname);
      exit (1);
    }
  }
  
  if (f != stdin)
    fclose (f);
  
  exit (1);
}

static void add_var (const char* name) {
  outbuf = realloc (outbuf, (++outcount) * sizeof(var_t));
  outbuf[outcount-1].seen = 0;
  outbuf[outcount-1].name = strdup (name);
  outbuf[outcount-1].descr = NULL;
}

static int add (const char* s) {
  size_t lnew = STRLEN(s), lold = STRLEN(outbuf[outcount-1].descr);

  if (lnew == 0)
    return (0);
  if (!outbuf[outcount-1].seen) {
    lold = 0;
    outbuf[outcount-1].seen = 1;
  }

  if (lold == 0)
    outbuf[outcount-1].descr = strdup (s);
  else {
    outbuf[outcount-1].descr = realloc (outbuf[outcount-1].descr, lold + lnew + 1);
    memcpy (&(outbuf[outcount-1].descr[lold-1])+1, s, lnew);
  }
  outbuf[outcount-1].descr[lold+lnew] = '\0';
  return (1);
}

static int add_c (int c) {
  char buf[2] = "\0\0";
  buf[0] = c;
  return (add (buf));
}

static void makedoc (FILE *in, FILE *out)
{
  char buffer[BUFFSIZE];
  char token[BUFFSIZE];
  char *p;
  int active = 0;
  int line = 0;
  int docstat = D_INIT;

  while ((fgets (buffer, sizeof (buffer), in)))
  {
    line++;
    if ((p = strchr (buffer, '\n')) == NULL)
    {
      fprintf (stderr, "%s: Line %d too long.  Ask a wizard to enlarge\n"
                       "%s: my buffer size.\n", Progname, line, Progname);
      exit (1);
    }
    else
      *p = '\0';

    if (!(p = get_token (token, sizeof (token), buffer)))
      continue;

    if (Debug)
    {
      fprintf (stderr, "%s: line %d.  first token: \"%s\".\n",
               Progname, line, token);
    }
    
    if (!strcmp (token, "/*++*/"))
      active = 1;
    else if (!strcmp (token, "/*--*/"))
    {
      docstat = flush_doc (docstat);
      active = 0;
    }
    else if (active && (!strcmp (token, "/**") || !strcmp (token, "**")))
      docstat = handle_docline (p, docstat);
    else if (active && !strcmp (token, "{"))
    {
      docstat = flush_doc (docstat);
      handle_confline (p);
    }
  }
  flush_doc (docstat);
  fputs ("\n", out);
  qsort (outbuf, outcount, sizeof (var_t), &var_cmp);
  for (line = 0; line < outcount; line++) {
    if (outbuf[line].descr) {
      fprintf (out, "%s\n", outbuf[line].descr);
      free (outbuf[line].descr);
    }
    free (outbuf[line].name);
  }
  free (outbuf);
}

/* skip whitespace */

static char *skip_ws (char *s)
{
  while (*s && isspace ((unsigned char) *s))
    s++;

  return s;
}

/* isolate a token */

static char single_char_tokens[] = "[]{},;|";

static char *get_token (char *d, size_t l, char *s)
{
  char *t;
  short is_quoted = 0;
  char *dd = d;

  if (Debug)
     fprintf (stderr, "%s: get_token called for `%s'.\n",
              Progname, s);
  
  s = skip_ws (s);

  if (Debug > 1)
    fprintf (stderr, "%s: argumet after skip_ws():  `%s'.\n",
             Progname, s);

  if (!*s)
  {
    if (Debug)
      fprintf (stderr, "%s: no more tokens on this line.\n", Progname);
    return NULL;
  }

  if (strchr (single_char_tokens, *s))
  {
    if (Debug)
    {
      fprintf (stderr, "%s: found single character token `%c'.\n",
               Progname, *s);
    }
    d[0] = *s++;
    d[1] = 0;
    return s;
  }

  if (*s == '"')
  {
    if (Debug)
    {
      fprintf (stderr, "%s: found quote character.\n", Progname);
    }
      
    s++;
    is_quoted = 1;
  }

  for (t = s; *t && --l > 0; t++)
  {
    if (*t == '\\' && !t[1])
      break;

    if (is_quoted && *t == '\\')
    {
      switch ((*d = *++t))
      {
        case 'n': *d = '\n'; break;
        case 't': *d = '\t'; break;
        case 'r': *d = '\r'; break;
        case 'a': *d = '\a'; break;
      }

      d++;
      continue;
    }

    if (is_quoted && *t == '"')
    {
      t++;
      break;
    }
    else if (!is_quoted && strchr (single_char_tokens, *t))
      break;
    else if (!is_quoted && isspace ((unsigned char) *t))
      break;
    else
      *d++ = *t;
  }

  *d = '\0';

  if (Debug)
  {
    fprintf (stderr, "%s: Got %stoken: `%s'.\n",
             Progname, is_quoted ? "quoted " : "", dd);
    fprintf (stderr, "%s: Remainder: `%s'.\n",
             Progname, t);
  }
  
  return t;
}


/**
 ** Configuration line parser
 ** 
 ** The following code parses a line from init.h which declares
 ** a configuration variable.
 **
 **/

/* note: the following enum must be in the same order as the
 * following string definitions!
 */

enum 
{
  DT_NONE = 0,
  DT_BOOL,
  DT_NUM,
  DT_STR,
  DT_PATH,
  DT_QUAD,
  DT_SORT,
  DT_RX,
  DT_MAGIC,
  DT_SYN,
  DT_ADDR
};

struct 
{
  char *machine;
  char *human;
}
types[] = 
{
  { "DT_NONE",  "-none-" },
  { "DT_BOOL",  "boolean" },
  { "DT_NUM",   "number" },
  { "DT_STR",   "string" },
  { "DT_PATH",  "path" },
  { "DT_QUAD",  "quadoption" },
  { "DT_SORT",  "sort order" },
  { "DT_RX",    "regular expression" },
  { "DT_MAGIC", "folder magic" },
  { "DT_SYN",   NULL },
  { "DT_ADDR",  "e-mail address" },
  { NULL, NULL }
};
    

static int buff2type (const char *s)
{
  int type;
  
  for (type = DT_NONE; types[type].machine; type++)
    if (!strcmp (types[type].machine, s))
        return type;
  
  return DT_NONE;
}

static const char *type2human (int type)
{
  return types[type].human;
}
static void handle_confline (char *s)
{
  char varname[BUFFSIZE];
  char buff[BUFFSIZE];
  char tmp[BUFFSIZE];
  int type;
  
  char val[BUFFSIZE];

  /* xxx - put this into an actual state machine? */

  /* variable name */
  if (!(s = get_token (varname, sizeof (varname), s))) return;
  
  /* comma */
  if (!(s = get_token (buff, sizeof (buff), s))) return;
    
  /* type */
  if (!(s = get_token (buff, sizeof (buff), s))) return;

  type = buff2type (buff);

  /* possibly a "|" or comma */
  if (!(s = get_token (buff, sizeof (buff), s))) return;

  if (!strcmp (buff, "|"))
  {
    if (Debug) fprintf (stderr, "%s: Expecting <subtype> <comma>.\n", Progname);
    /* ignore subtype and comma */
    if (!(s = get_token (buff, sizeof (buff), s))) return;
    if (!(s = get_token (buff, sizeof (buff), s))) return;
  }

  /* redraw, comma */
  
  while (1)
  {
    if (!(s = get_token (buff, sizeof (buff), s))) return;
    if (!strcmp (buff, ","))
      break;
  }

  /* option name or UL &address */
  if (!(s = get_token (buff, sizeof (buff), s))) return;
  if (!strcmp (buff, "UL"))
    if (!(s = get_token (buff, sizeof (buff), s))) return;

  /* comma */
  if (!(s = get_token (buff, sizeof (buff), s))) return;

  if (Debug) fprintf (stderr, "%s: Expecting default value.\n", Progname);
  
  /* <default value> or UL <default value> */
  if (!(s = get_token (buff, sizeof (buff), s))) return;
  if (!strcmp (buff, "UL"))
  {
    if (Debug) fprintf (stderr, "%s: Skipping UL.\n", Progname);
    if (!(s = get_token (buff, sizeof (buff), s))) return;
  }

  memset (tmp, 0, sizeof (tmp));

  do 
  {
    if (!strcmp (buff, "}"))
      break;

    strncpy (tmp + STRLEN(tmp), buff, sizeof (tmp) - STRLEN(tmp));
  }
  while ((s = get_token (buff, sizeof (buff), s)));

  pretty_default (val, sizeof (val), tmp, type);
  add_var (varname);
  print_confline (varname, type, val);
}

static void pretty_default (char *t, size_t l, const char *s, int type)
{
  memset (t, 0, l);
  l--;

  switch (type)
  {
    case DT_QUAD:
    {    
      if (!strcasecmp (s, "M_YES")) strncpy (t, "yes", l);
      else if (!strcasecmp (s, "M_NO")) strncpy (t, "no", l);
      else if (!strcasecmp (s, "M_ASKYES")) strncpy (t, "ask-yes", l);
      else if (!strcasecmp (s, "M_ASKNO")) strncpy (t, "ask-no", l);
      break;
    }
    case DT_BOOL:
    {
      if (atoi (s))
        strncpy (t, "yes", l);
      else
        strncpy (t, "no", l);
      break;
    }
    case DT_SORT:
    {
      /* heuristic! */
      strncpy (t, s + 5, l);
      for (; *t; t++) *t = tolower ((unsigned char) *t);
      break;
    }
    case DT_MAGIC:
    {
      /* heuristic! */
      strncpy (t, s + 2, l);
      for (; *t; t++) *t = tolower ((unsigned char) *t);
      break;
    }
    case DT_STR:
    case DT_RX:
    case DT_ADDR:
    case DT_PATH:
    {
      if (!strcmp (s, "0"))
        break;
      /* fallthrough */
    }
    default:
    {
      strncpy (t, s, l);
      break;
    }
  }
}

static void char_to_escape (char *dest, unsigned int c)
{
  switch (c)
  {
    case '\r': strcpy (dest, "\\r"); break;        /* __STRCPY_CHECKED__ */
    case '\n': strcpy (dest, "\\n"); break;        /* __STRCPY_CHECKED__ */
    case '\t': strcpy (dest, "\\t"); break;        /* __STRCPY_CHECKED__ */
    case '\f': strcpy (dest, "\\f"); break;        /* __STRCPY_CHECKED__ */
    default: sprintf (dest, "\\%03o", c); break;
  }
}
static void conf_char_to_escape (unsigned int c)
{
  char buff[16];
  char_to_escape (buff, c);
  add (buff);
}

static void conf_print_strval (const char *v)
{
  for (; *v; v++)
  {
    if (*v < ' ' || *v & 0x80)
    {
      conf_char_to_escape ((unsigned int) *v);
      continue;
    }

    if (*v == '"'  || *v == '\\')
      add_c ('\\');
    add_c (*v);
  }
}

static void man_print_strval (const char *v)
{
  for (; *v; v++)
  {
    if (*v < ' ' || *v & 0x80)
    {
      add_c ('\\');
      conf_char_to_escape ((unsigned int) *v);
      continue;
    }
    
    if (*v == '"')
      add ("\\(rq");
    else if (*v == '\\')
      add ("\\\\");
    else
      add_c (*v);
  }
}

static void sgml_print_strval (const char *v)
{
  char buff[16];
  for (; *v; v++)
  {
    if (*v <  ' ' || *v & 0x80)
    {
      char_to_escape (buff, (unsigned int) *v);
      sgml_fputs (buff);
      continue;
    }
    sgml_fputc ((unsigned int) *v);
  }
}

static int sgml_fputc (int c)
{
  switch (c)
  {
    case '<': return add ("&lt;");
    case '>': return add ("&gt;");
    case '$': return add ("&dollar;");
    case '_': return add ("&lowbar;");
    case '%': return add ("&percnt;");
    case '&': return add ("&amp;");
    case '\\': return add ("&bsol;");
    case '"': return add ("&dquot;");
    case '[': return add ("&lsqb;");
    case ']': return add ("&rsqb;");
    case '~': return add ("&tilde;");
    default:  return add_c (c);
  }
}

static int sgml_fputs (const char *s)
{
  for (; *s; s++)
    if (sgml_fputc ((unsigned int) *s) == EOF)
      return EOF;
  
  return 0;
}

static void print_confline (const char *varname, int type, const char *val)
{
  if (type == DT_SYN) return;
  
  switch (OutputFormat)
  {
    /* configuration file */
    case F_CONF:
    {
      if (type == DT_STR || type == DT_RX || type == DT_ADDR || type == DT_PATH)
      {
        add ("\n# set ");
        add (varname);
        add ("\"");
        conf_print_strval (val);
        add ("\"");
      }
      else if (type != DT_SYN)
      {
        add ("\n# set ");
        add (varname);
        add ("=");
        add (val);
      }
      
      add ("\n#\n# Name: ");
      add (varname);
      add ("\n# Type: ");
      add (type2human (type));
      if (type == DT_STR || type == DT_RX || type == DT_ADDR || type == DT_PATH)
      {
        add ("\n# Default: \"");
        conf_print_strval (val);
        add ("\"");
      }
      else
      {
        add ("\n# Default: ");
        add (val);
      }

      add ("\n# ");
      break;
    }

    /* manual page */
    case F_MAN:
    {
      add (".TP\n.B ");
      add (varname);
      add ("\n.nf\n");
      add ("Type: ");
      add (type2human (type));
      add_c ('\n');
      if (type == DT_STR || type == DT_RX || type == DT_ADDR || type == DT_PATH)
      {
        add ("Default: \\(lq");
        man_print_strval (val);
        add ("\\(rq\n");
      }
      else {
        add ("Default: ");
        add (val);
        add_c ('\n');
      }
      add (".fi");

      break;
    }
    
    /* SGML based manual */
    case F_SGML:
    {
      add ("\n<sect2>");
      sgml_fputs (varname);
      add ("<label id=\"");
      add (varname);
      add ("\">");
      add ("\n<p>\nType: <tt>");
      add (type2human (type));
      add ("</tt>\n\n");

      if (type == DT_STR || type == DT_RX || type == DT_ADDR || type == DT_PATH)
      {
        add ("<p>\nDefault: <tt>&dquot;");
        sgml_print_strval (val);
        add ("&dquot;</tt>");
      }
      else
      {
        add ("<p>\nDefault: <tt>");
        add (val);
        add ("</tt>");
      }
      break;
    }
    /* make gcc happy */
    default:
      break;
  }
}

/**
 ** Documentation line parser
 **
 ** The following code parses specially formatted documentation 
 ** comments in init.h.
 **
 ** The format is very remotely inspired by nroff. Most important, it's
 ** easy to parse and convert, and it was easy to generate from the SGML 
 ** source of mutt's original manual.
 **
 ** - \fI switches to italics
 ** - \fB switches to boldface
 ** - \fP switches to normal display
 ** - .dl on a line starts a definition list (name taken taken from HTML).
 ** - .dt starts a term in a definition list.
 ** - .dd starts a definition in a definition list.
 ** - .de on a line finishes a definition list.
 ** - .ts on a line starts a "tscreen" environment (name taken from SGML).
 ** - .te on a line finishes this environment.
 ** - .pp on a line starts a paragraph.
 ** - \$word will be converted to a reference to word, where appropriate.
 **   Note that \$$word is possible as well.
 ** - '. ' in the beginning of a line expands to two space characters.
 **   This is used to protect indentations in tables.
 **/

/* close eventually-open environments. */

static int fd_recurse = 0;

static int flush_doc (int docstat)
{
  if (docstat & D_INIT)
    return D_INIT;

  if (fd_recurse++)
  {
    fprintf (stderr, "%s: Internal error, recursion in flush_doc()!\n", Progname);
    exit (1);
  }

  if (docstat & (D_TAB))
    docstat = print_it (SP_END_TAB, NULL, docstat);

  if (docstat & (D_DL))
    docstat = print_it (SP_END_DL, NULL, docstat);

  if (docstat & (D_EM | D_BF))
    docstat = print_it (SP_END_FT, NULL, docstat);

  docstat = print_it (SP_NEWLINE, NULL, 0);

  fd_recurse--;
  return D_INIT;
}

/* print something. */

static int print_it (int special, char *str, int docstat)
{
  int onl = docstat & (D_NL|D_NP);
  
  docstat &= ~(D_NL|D_NP|D_INIT);

  switch (OutputFormat)
  {
    /* configuration file */
    case F_CONF:
    {
      switch (special)
      {
        static int Continuation = 0;

        case SP_END_FT: docstat &= ~(D_EM|D_BF); break;
        case SP_START_BF: docstat |= D_BF; break;
        case SP_START_EM: docstat |= D_EM; break;
        case SP_NEWLINE: 
        {
          if (onl)
            docstat |= onl;
          else
          {
            add ("\n# ");
            docstat |= D_NL;
          }
          if (docstat & D_DL)
            ++ Continuation;
          break;
        }
        case SP_NEWPAR:
        {
          if (onl & D_NP)
          {
            docstat |= onl;
            break;
          }

          if (!(onl & D_NL))
            add ("\n# ");
          add ("\n# ");
          docstat |= D_NP;
          break;
        }
        case SP_START_TAB: 
        {
          if (!onl) 
            add ("\n# ");
          docstat |= D_TAB;
          break;
        }
        case SP_END_TAB:
        {
          docstat &= ~D_TAB;
          docstat |= D_NL;
          break;
        }
        case SP_START_DL:
        {
          docstat |= D_DL;
          break;
        }
        case SP_DT:
        {
          Continuation = 0;
          docstat |= D_DT;
          break;
        }
        case SP_DD:
        {
          Continuation = 0;
          break;
        }
        case SP_END_DL:
        {
          Continuation = 0;
          docstat &= ~D_DL;
          break;
        }
        case SP_STR:
        {
          if (Continuation)
          {
            Continuation = 0;
            add ("        ");
          }
          add (str);
          if (docstat & D_DT)
          { 
            int i;

            for (i = STRLEN(str) ; i < 8 ; i++)
              add_c (' ');
            docstat &= ~D_DT;
            docstat |= D_NL;
          }
          break;
        }
      }
      break;
    }

    /* manual page */
    case F_MAN:
    {
      switch (special)
      {
        case SP_END_FT: 
        {
          add ("\\fP");
          docstat &= ~(D_EM|D_BF);
          break;
        }
        case SP_START_BF: 
        {
          add ("\\fB");
          docstat |= D_BF;
          docstat &= ~D_EM;
          break;
        }
        case SP_START_EM:
        {
          add ("\\fI");
          docstat |= D_EM;
          docstat &= ~D_BF;
          break;
        }
        case SP_NEWLINE:
        {
          if (onl)
            docstat |= onl;
          else
          {
            add_c ('\n');
            docstat |= D_NL;
          }
          break;
        }
        case SP_NEWPAR:
        {
          if (onl & D_NP)
          {
            docstat |= onl;
            break;
          }

          if (!(onl & D_NL))
            add_c ('\n');
          add (".IP\n");

          docstat |= D_NP;
          break;
        }
        case SP_START_TAB:
        {
          add ("\n.IP\n.DS\n.sp\n.ft CR\n.nf\n");
          docstat |= D_TAB | D_NL;
          break;
        }
        case SP_END_TAB:
        {
          add ("\n.fi\n.ec\n.ft P\n.sp\n");
          docstat &= ~D_TAB;
          docstat |= D_NL;
          break;
        }
        case SP_START_DL:
        {
          add ("\n.RS");
          docstat |= D_DL;
          break;
        }
        case SP_DT:
        {
          add ("\n.IP ");
          break;
        }
        case SP_DD:
        {
          add ("\n");
          break;
        }
        case SP_END_DL:
        {
          add ("\n.RE");
          docstat &= ~D_DL;
          break;
        }
        case SP_STR:
        {
          while (*str)
          {
            for (; *str; str++)
            {
              if (*str == '"')
                add ("\\(rq");
              else if (*str == '\\')
                add ("\\\\");
              else if (!strncmp (str, "``", 2))
              {
                add ("\\(lq");
                str++;
              }
              else if (!strncmp (str, "''", 2))
              {
                add ("\\(rq");
                str++;
              }
              else
                add_c (*str);
            }
          }
          break;
        }
      }
      break;
    }

    /* SGML based manual */
    case F_SGML:
    {
      switch (special)
      {
        case SP_END_FT: 
        {
          if (docstat & D_EM) add("</em>");
          if (docstat & D_BF) add("</bf>");
          docstat &= ~(D_EM|D_BF);
          break;
        }
        case SP_START_BF: 
        {
          add ("<bf>");
          docstat |= D_BF;
          docstat &= ~D_EM;
          break;
        }
        case SP_START_EM:
        {
          add ("<em>");
          docstat |= D_EM;
          docstat &= ~D_BF;
          break;
        }
        case SP_NEWLINE:
        {
          if (onl)
            docstat |= onl;
          else
          {
            add ("\n");
                   docstat |= D_NL;
          }
          break;
        }
        case SP_NEWPAR:
        {
          if (onl & D_NP)
          {
            docstat |= onl;
            break;
          }

          if (!(onl & D_NL))
            add ("\n");
          add ("\n<p>\n");

          docstat |= D_NP;
          break;
        }
        case SP_START_TAB:
        {
          add ("\n<tscreen><verb>\n");
          docstat |= D_TAB | D_NL;
          break;
        }
        case SP_END_TAB:
        {
          add ("\n</verb></tscreen>");
          docstat &= ~D_TAB;
          docstat |= D_NL;
          break;
        }
        case SP_START_DL:
        {
          add ("\n<descrip>\n");
          docstat |= D_DL;
          break;
        }
        case SP_DT:
        {
          add ("<tag>");
          break;
        }
        case SP_DD:
        {
          add ("</tag>");
          break;
        }
        case SP_END_DL:
        {
          add ("</descrip>\n");
          docstat &= ~D_DL;
          break;
        }
        case SP_STR:
        {
          if (docstat & D_TAB)
            add (str);
          else
            sgml_fputs (str);
          break;
        }
      }
      break;
    }
    /* make gcc happy (unreached) */
    default:
      break;
  }

  return docstat;
}

void print_ref (int output_dollar, const char *ref)
{
  switch (OutputFormat)
  {
  case F_CONF:
  case F_MAN:
    if (output_dollar)
      add_c ('$');
    add (ref);
    break;

  case F_SGML:
    add ("<ref id=\"");
    add (ref);
    add ("\" name=\"");
    if (output_dollar)
      add ("&dollar;");
    sgml_fputs (ref);
    add ("\">");
    break;

  default:
    break;
  }
}

static int commit_buff (char *buff, char **d, int docstat)
{
  if (*d > buff)
  {
    **d = '\0';
    docstat = print_it (SP_STR, buff, docstat);
    *d = buff;
  }

  return docstat;
}

static int handle_docline (char *l, int docstat)
{
  char buff[BUFFSIZE];
  char *s, *d;
  l = skip_ws (l);

  if (Debug)
    fprintf (stderr, "%s: handle_docline `%s'\n", Progname, l);
  
  if (!strncmp (l, ".pp", 3))
    return print_it (SP_NEWPAR, NULL, docstat);
  else if (!strncmp (l, ".ts", 3))
    return print_it (SP_START_TAB, NULL, docstat);
  else if (!strncmp (l, ".te", 3))
    return print_it (SP_END_TAB, NULL, docstat);
  else if (!strncmp (l, ".dl", 3))
    return print_it (SP_START_DL, NULL, docstat);
  else if (!strncmp (l, ".de", 3))
    return print_it (SP_END_DL, NULL, docstat);
  else if (!strncmp (l, ". ", 2))
    *l = ' ';

  for (s = l, d = buff; *s; s++)
  {
    if (!strncmp (s, "\\(as", 4))
    {
      *d++ = '*';
      s += 3;
    }
    else if (!strncmp (s, "\\(rs", 4))
    {
      *d++ = '\\';
      s += 3;
    }
    else if (!strncmp (s, "\\fI", 3))
    {
      docstat = commit_buff (buff, &d, docstat);
      docstat = print_it (SP_START_EM, NULL, docstat);
      s += 2;
    }
    else if (!strncmp (s, "\\fB", 3))
    {
      docstat = commit_buff (buff, &d, docstat);
      docstat = print_it (SP_START_BF, NULL, docstat);
      s += 2;
    }
    else if (!strncmp (s, "\\fP", 3))
    {
      docstat = commit_buff (buff, &d, docstat);
      docstat = print_it (SP_END_FT, NULL, docstat);
      s += 2;
    }
    else if (!strncmp (s, ".dt", 3))
    {
      docstat = commit_buff (buff, &d, docstat);
      docstat = print_it (SP_DT, NULL, docstat);
      s += 3;
    }
    else if (!strncmp (s, ".dd", 3))
    {
      docstat = commit_buff (buff, &d, docstat);
      docstat = print_it (SP_DD, NULL, docstat);
      s += 3;
    }
    else if (*s == '$')
    {
      int output_dollar = 0;
      char *ref;
      char save;

      ++s;
      if (*s == '$')
      {
        output_dollar = 1;
        ++s;
      }
      if (*s == '$')
      {
        *d++ = '$';
      }
      else
      {
        ref = s;
        while (isalnum ((unsigned char) *s) || *s == '-' || *s == '_')
          ++s;

        docstat = commit_buff (buff, &d, docstat);
        save = *s;
        *s = 0;
        print_ref (output_dollar, ref);
        *s = save;
        --s;
      }
    }
    else
      *d++ = *s;
  }

  docstat = commit_buff (buff, &d, docstat);
  return print_it (SP_NEWLINE, NULL, docstat);
}
