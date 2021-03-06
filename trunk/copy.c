/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2000,2002 Michael R. Elkins <me@mutt.org>
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
#include "handler.h"
#include "mx.h"
#include "copy.h"
#include "rfc2047.h"
#include "mime.h"
#include "mutt_crypt.h"
#include "mutt_idna.h"

#include "lib/mem.h"
#include "lib/str.h"
#include "lib/debug.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>             /* needed for SEEK_SET under SunOS 4.1.4 */

static int address_header_decode (char **str);
static int copy_delete_attach (BODY * b, FILE * fpin, FILE * fpout,
                               char *date);

/* Ok, the only reason for not merging this with mutt_copy_header()
 * below is to avoid creating a HEADER structure in message_handler().
 */
int
mutt_copy_hdr (FILE* in, FILE* out, LOFF_T off_start, LOFF_T off_end,
               int flags, const char *prefix) {
  int from = 0;
  int this_is_from;
  int ignore = 0;
  char buf[STRING];             /* should be long enough to get most fields in one pass */
  char *nl;
  LIST *t;
  char **headers;
  int hdr_count;
  int x;
  char *this_one = NULL;
  int error;
  int curline = 0;

  if (ftello (in) != off_start)
    fseeko (in, off_start, 0);

  buf[0] = '\n';
  buf[1] = 0;

  if ((flags &
       (CH_REORDER | CH_WEED | CH_MIME | CH_DECODE | CH_PREFIX |
        CH_WEED_DELIVERED)) == 0) {
    /* Without these flags to complicate things
     * we can do a more efficient line to line copying
     */
    while (ftello (in) < off_end) {
      nl = strchr (buf, '\n');

      if ((fgets (buf, sizeof (buf), in)) == NULL)
        break;

      /* Is it the begining of a header? */
      if (nl && buf[0] != ' ' && buf[0] != '\t') {
        ignore = 1;
        if (!from && str_ncmp ("From ", buf, 5) == 0) {
          if ((flags & CH_FROM) == 0)
            continue;
          from = 1;
        }
        else if (flags & (CH_NOQFROM) &&
                 ascii_strncasecmp (">From ", buf, 6) == 0)
          continue;

        else if (buf[0] == '\n' || (buf[0] == '\r' && buf[1] == '\n'))
          break;                /* end of header */

        if ((flags & (CH_UPDATE | CH_XMIT | CH_NOSTATUS)) &&
            (ascii_strncasecmp ("Status:", buf, 7) == 0 ||
             ascii_strncasecmp ("X-Status:", buf, 9) == 0))
          continue;
        if ((flags & (CH_UPDATE_LEN | CH_XMIT | CH_NOLEN)) &&
            (ascii_strncasecmp ("Content-Length:", buf, 15) == 0 ||
             ascii_strncasecmp ("Lines:", buf, 6) == 0))
          continue;
        if ((flags & CH_UPDATE_REFS) &&
            ascii_strncasecmp ("References:", buf, 11) == 0)
          continue;
        if ((flags & CH_UPDATE_IRT) &&
            ascii_strncasecmp ("In-Reply-To:", buf, 12) == 0)
          continue;
        ignore = 0;
      }

      if (!ignore && fputs (buf, out) == EOF)
        return (-1);
    }
    return 0;
  }

  hdr_count = 1;
  x = 0;
  error = FALSE;

  /* We are going to read and collect the headers in an array
   * so we are able to do re-ordering.
   * First count the number of entries in the array
   */
  if (flags & CH_REORDER) {
    for (t = HeaderOrderList; t; t = t->next) {
      debug_print (1, ("Reorder list: %s\n", t->data));
      hdr_count++;
    }
  }

  debug_print (1, ("WEED is %s\n", (flags & CH_WEED) ? "Set" : "Not"));

  headers = mem_calloc (hdr_count, sizeof (char *));

  /* Read all the headers into the array */
  while (ftello (in) < off_end) {
    nl = strchr (buf, '\n');

    /* Read a line */
    if ((fgets (buf, sizeof (buf), in)) == NULL)
      break;

    /* Is it the begining of a header? */
    if (nl && buf[0] != ' ' && buf[0] != '\t') {

      /* set curline to 1 for To:/Cc:/Bcc: and 0 otherwise */
      curline = (flags & CH_WEED) && (ascii_strncmp ("To:", buf, 3) == 0 ||
                                      ascii_strncmp ("Cc:", buf, 3) == 0 ||
                                      ascii_strncmp ("Bcc:", buf, 4) == 0);

      /* Do we have anything pending? */
      if (this_one) {
        if (flags & CH_DECODE) {
          if (!address_header_decode (&this_one))
            rfc2047_decode (&this_one);
        }

        if (!headers[x])
          headers[x] = this_one;
        else {
          mem_realloc (&headers[x], str_len (headers[x]) +
                        str_len (this_one) + sizeof (char));
          strcat (headers[x], this_one);        /* __STRCAT_CHECKED__ */
          mem_free (&this_one);
        }

        this_one = NULL;
      }

      ignore = 1;
      this_is_from = 0;
      if (!from && str_ncmp ("From ", buf, 5) == 0) {
        if ((flags & CH_FROM) == 0)
          continue;
        this_is_from = from = 1;
      }
      else if (buf[0] == '\n' || (buf[0] == '\r' && buf[1] == '\n'))
        break;                  /* end of header */

      /* note: CH_FROM takes precedence over header weeding. */
      if (!((flags & CH_FROM) && (flags & CH_FORCE_FROM) && this_is_from) &&
          (flags & CH_WEED) &&
          mutt_matches_ignore (buf, Ignore) &&
          !mutt_matches_ignore (buf, UnIgnore))
        continue;
      if ((flags & CH_WEED_DELIVERED) &&
          ascii_strncasecmp ("Delivered-To:", buf, 13) == 0)
        continue;
      if ((flags & (CH_UPDATE | CH_XMIT | CH_NOSTATUS)) &&
          (ascii_strncasecmp ("Status:", buf, 7) == 0 ||
           ascii_strncasecmp ("X-Status:", buf, 9) == 0))
        continue;
      if ((flags & (CH_UPDATE_LEN | CH_XMIT | CH_NOLEN)) &&
          (ascii_strncasecmp ("Content-Length:", buf, 15) == 0 ||
           ascii_strncasecmp ("Lines:", buf, 6) == 0))
        continue;
      if ((flags & CH_MIME) &&
          ((ascii_strncasecmp ("content-", buf, 8) == 0 &&
            (ascii_strncasecmp ("transfer-encoding:", buf + 8, 18) == 0 ||
             ascii_strncasecmp ("type:", buf + 8, 5) == 0)) ||
           ascii_strncasecmp ("mime-version:", buf, 13) == 0))
        continue;
      if ((flags & CH_UPDATE_REFS) &&
          ascii_strncasecmp ("References:", buf, 11) == 0)
        continue;
      if ((flags & CH_UPDATE_IRT) &&
          ascii_strncasecmp ("In-Reply-To:", buf, 12) == 0)
        continue;

      /* Find x -- the array entry where this header is to be saved */
      if (flags & CH_REORDER) {
        for (t = HeaderOrderList, x = 0; (t); t = t->next, x++) {
          if (!ascii_strncasecmp (buf, t->data, str_len (t->data))) {
            debug_print (2, ("Reorder: %s matches %s\n", t->data, buf));
            break;
          }
        }
      }

      ignore = 0;
    }                           /* If beginning of header */

    if (!ignore) {
      debug_print (2, ("Reorder: x = %d; hdr_count = %d\n", x, hdr_count));
      if (!this_one)
        this_one = str_dup (buf);
      /* we do want to see all lines if this header doesn't feature
       * abbreviations (curline is 0), $max_display_recips is 0 and
       * while the number hasn't reached $max_display_recips yet */
      else if (curline == 0 || MaxDispRecips == 0 || ++curline <= MaxDispRecips) {
        mem_realloc (&this_one,
                      str_len (this_one) + str_len (buf) +
                      sizeof (char));
        strcat (this_one, buf); /* __STRCAT_CHECKED__ */
      /* only for the first line which doesn't exeeds
       * $max_display_recips: abbreviate it */
      } else if (curline == MaxDispRecips+1) {
        mem_realloc (&this_one, str_len (this_one) + 5);
        strcat (this_one, " ...");
      }
    }
  }                             /* while (ftello (in) < off_end) */

  /* Do we have anything pending?  -- XXX, same code as in above in the loop. */
  if (this_one) {
    if (flags & CH_DECODE) {
      if (!address_header_decode (&this_one))
        rfc2047_decode (&this_one);
    }

    if (!headers[x])
      headers[x] = this_one;
    else {
      mem_realloc (&headers[x], str_len (headers[x]) +
                    str_len (this_one) + sizeof (char));
      strcat (headers[x], this_one);    /* __STRCAT_CHECKED__ */
      mem_free (&this_one);
    }

    this_one = NULL;
  }

  /* Now output the headers in order */
  for (x = 0; x < hdr_count; x++) {
    if (headers[x]) {
#if 0
      if (flags & CH_DECODE)
        rfc2047_decode (&headers[x]);
#endif

      /* We couldn't do the prefixing when reading because RFC 2047
       * decoding may have concatenated lines.
       */
      if (flags & CH_PREFIX) {
        char *ch = headers[x];
        int print_prefix = 1;

        while (*ch) {
          if (print_prefix) {
            if (fputs (prefix, out) == EOF) {
              error = TRUE;
              break;
            }
            print_prefix = 0;
          }

          if (*ch == '\n' && ch[1])
            print_prefix = 1;

          if (putc (*ch++, out) == EOF) {
            error = TRUE;
            break;
          }
        }
        if (error)
          break;
      }
      else {
        if (fputs (headers[x], out) == EOF) {
          error = TRUE;
          break;
        }
      }
    }
  }

  /* Free in a separate loop to be sure that all headers are freed
   * in case of error. */
  for (x = 0; x < hdr_count; x++)
    mem_free (&headers[x]);
  mem_free (&headers);

  if (error)
    return (-1);
  return (0);
}

/* flags
 	CH_DECODE	RFC2047 header decoding
 	CH_FROM		retain the "From " message separator
        CH_FORCE_FROM	give CH_FROM precedence over CH_WEED
 	CH_MIME		ignore MIME fields
	CH_NOLEN	don't write Content-Length: and Lines:
 	CH_NONEWLINE	don't output a newline after the header
 	CH_NOSTATUS	ignore the Status: and X-Status:
 	CH_PREFIX	quote header with $indent_str
 	CH_REORDER	output header in order specified by `hdr_order'
  	CH_TXTPLAIN	generate text/plain MIME headers [hack alert.]
 	CH_UPDATE	write new Status: and X-Status:
 	CH_UPDATE_LEN	write new Content-Length: and Lines:
 	CH_XMIT		ignore Lines: and Content-Length:
 	CH_WEED		do header weeding
	CH_NOQFROM      ignore ">From " line
	CH_UPDATE_IRT	update the In-Reply-To: header
	CH_UPDATE_REFS	update the References: header

   prefix
   	string to use if CH_PREFIX is set
 */

int
mutt_copy_header (FILE * in, HEADER * h, FILE * out, int flags,
                  const char *prefix)
{
  char buffer[SHORT_STRING];

  if (h->env)
    flags |= (h->env->irt_changed ? CH_UPDATE_IRT : 0) |
      (h->env->refs_changed ? CH_UPDATE_REFS : 0);

  if (mutt_copy_hdr (in, out, h->offset, h->content->offset, flags, prefix) ==
      -1)
    return (-1);

  if (flags & CH_TXTPLAIN) {
    char chsbuf[SHORT_STRING];

    fputs ("MIME-Version: 1.0\n", out);
    fputs ("Content-Transfer-Encoding: 8bit\n", out);
    fputs ("Content-Type: text/plain; charset=", out);
    mutt_canonical_charset (chsbuf, sizeof (chsbuf),
                            Charset ? Charset : "us-ascii");
    rfc822_cat (buffer, sizeof (buffer), chsbuf, MimeSpecials);
    fputs (buffer, out);
    fputc ('\n', out);

    if (ferror (out) != 0 || feof (out) != 0)
      return -1;

  }

  if (flags & CH_UPDATE) {
    if ((flags & CH_NOSTATUS) == 0) {
      if (h->env->irt_changed && h->env->in_reply_to) {
        LIST *listp = h->env->in_reply_to;

        if (fputs ("In-Reply-To: ", out) == EOF)
          return (-1);

        for (; listp; listp = listp->next)
          if ((fputs (listp->data, out) == EOF) || (fputc (' ', out) == EOF))
            return (-1);

        if (fputc ('\n', out) == EOF)
          return (-1);
      }

      if (h->env->refs_changed && h->env->references) {
        LIST *listp = h->env->references, *refs = NULL, *t;

        if (fputs ("References: ", out) == EOF)
          return (-1);

        /* Mutt stores references in reverse order, thus we create
         * a reordered refs list that we can put in the headers */
        for (; listp; listp = listp->next, refs = t) {
          t = (LIST *) mem_malloc (sizeof (LIST));
          t->data = listp->data;
          t->next = refs;
        }

        for (; refs; refs = refs->next)
          if ((fputs (refs->data, out) == EOF) || (fputc (' ', out) == EOF))
            return (-1);

        /* clearing refs from memory */
        for (t = refs; refs; refs = t->next, t = refs)
          mem_free (&refs);

        if (fputc ('\n', out) == EOF)
          return (-1);
      }

      if (h->old || h->read) {
        if (fputs ("Status: ", out) == EOF)
          return (-1);

        if (h->read) {
          if (fputs ("RO", out) == EOF)
            return (-1);
        }
        else if (h->old) {
          if (fputc ('O', out) == EOF)
            return (-1);
        }

        if (fputc ('\n', out) == EOF)
          return (-1);
      }

      if (h->flagged || h->replied) {
        if (fputs ("X-Status: ", out) == EOF)
          return (-1);

        if (h->replied) {
          if (fputc ('A', out) == EOF)
            return (-1);
        }

        if (h->flagged) {
          if (fputc ('F', out) == EOF)
            return (-1);
        }

        if (fputc ('\n', out) == EOF)
          return (-1);
      }
    }
  }

  if (flags & CH_UPDATE_LEN && (flags & CH_NOLEN) == 0) {
    fprintf (out, "Content-Length: " OFF_T_FMT "\n", h->content->length);
    if (h->lines != 0 || h->content->length == 0)
      fprintf (out, "Lines: %d\n", h->lines);
  }

  if ((flags & CH_NONEWLINE) == 0) {
    if (flags & CH_PREFIX)
      fputs (prefix, out);
    if (fputc ('\n', out) == EOF)       /* add header terminator */
      return (-1);
  }

  if (ferror (out) || feof (out))
    return -1;

  return (0);
}

/* Count the number of lines and bytes to be deleted in this body*/
static int count_delete_lines (FILE * fp, BODY * b, LOFF_T *length,
                               size_t datelen)
{
  int dellines = 0;
  long l;
  int ch;

  if (b->deleted) {
    fseeko (fp, b->offset, SEEK_SET);
    for (l = b->length; l; l--) {
      ch = getc (fp);
      if (ch == EOF)
        break;
      if (ch == '\n')
        dellines++;
    }
    dellines -= 3;
    *length -= b->length - (84 + datelen);
    /* Count the number of digits exceeding the first one to write the size */
    for (l = 10; b->length >= l; l *= 10)
      (*length)++;
  }
  else {
    for (b = b->parts; b; b = b->next)
      dellines += count_delete_lines (fp, b, length, datelen);
  }
  return dellines;
}

/* make a copy of a message
 * 
 * fpout	where to write output
 * fpin		where to get input
 * hdr		header of message being copied
 * body		structure of message being copied
 * flags
 * 	M_CM_NOHEADER	don't copy header
 * 	M_CM_PREFIX	quote header and body
 *	M_CM_DECODE	decode message body to text/plain
 *	M_CM_DISPLAY	displaying output to the user
 *      M_CM_PRINTING   printing the message
 *	M_CM_UPDATE	update structures in memory after syncing
 *	M_CM_DECODE_PGP	used for decoding PGP messages
 *	M_CM_CHARCONV	perform character set conversion 
 * chflags	flags to mutt_copy_header()
 */

int
_mutt_copy_message (FILE * fpout, FILE * fpin, HEADER * hdr, BODY * body,
                    int flags, int chflags)
{
  char prefix[SHORT_STRING];
  STATE s;
  LOFF_T new_offset = -1;
  int rc = 0;

  if (flags & M_CM_PREFIX) {
    if (option (OPTTEXTFLOWED))
      strfcpy (prefix, ">", sizeof (prefix));
    else
      _mutt_make_string (prefix, sizeof (prefix), NONULL (Prefix), Context,
                         hdr, 0);
  }

  if ((flags & M_CM_NOHEADER) == 0) {
    if (flags & M_CM_PREFIX)
      chflags |= CH_PREFIX;

    else if (hdr->attach_del && (chflags & CH_UPDATE_LEN)) {
      int new_lines;
      LOFF_T new_length = body->length;
      char date[SHORT_STRING];

      mutt_make_date (date, sizeof (date));
      date[5] = date[str_len (date) - 1] = '\"';

      /* Count the number of lines and bytes to be deleted */
      fseeko (fpin, body->offset, SEEK_SET);
      new_lines = hdr->lines -
        count_delete_lines (fpin, body, &new_length, str_len (date));

      /* Copy the headers */
      if (mutt_copy_header (fpin, hdr, fpout,
                            chflags | CH_NOLEN | CH_NONEWLINE, NULL))
        return -1;
      fprintf (fpout, "Content-Length: " OFF_T_FMT "\n", new_length);
      if (new_lines <= 0)
        new_lines = 0;
      else
        fprintf (fpout, "Lines: %d\n\n", new_lines);
      if (ferror (fpout) || feof (fpout))
        return -1;
      new_offset = ftello (fpout);

      /* Copy the body */
      fseeko (fpin, body->offset, SEEK_SET);
      if (copy_delete_attach (body, fpin, fpout, date))
        return -1;

#ifdef DEBUG
      {
        LOFF_T fail = ((ftello (fpout) - new_offset) - new_length);

        if (fail) {
          mutt_error ("The length calculation was wrong by %ld bytes", fail);
          new_length += fail;
          mutt_sleep (1);
        }
      }
#endif

      /* Update original message if we are sync'ing a mailfolder */
      if (flags & M_CM_UPDATE) {
        hdr->attach_del = 0;
        hdr->lines = new_lines;
        body->offset = new_offset;

        /* update the total size of the mailbox to reflect this deletion */
        Context->size -= body->length - new_length;
        /*
         * if the message is visible, update the visible size of the mailbox
         * as well.
         */
        if (Context->v2r[hdr->msgno] != -1)
          Context->vsize -= body->length - new_length;

        body->length = new_length;
        mutt_free_body (&body->parts);
      }

      return 0;
    }

    if (mutt_copy_header (fpin, hdr, fpout, chflags,
                          (chflags & CH_PREFIX) ? prefix : NULL) == -1)
      return -1;

    new_offset = ftello (fpout);
  }

  if (flags & M_CM_DECODE) {
    /* now make a text/plain version of the message */
    memset (&s, 0, sizeof (STATE));
    s.fpin = fpin;
    s.fpout = fpout;
    if (flags & M_CM_PREFIX)
      s.prefix = prefix;
    if (flags & M_CM_DISPLAY)
      s.flags |= M_DISPLAY;
    if (flags & M_CM_PRINTING)
      s.flags |= M_PRINTING;
    if (flags & M_CM_WEED)
      s.flags |= M_WEED;
    if (flags & M_CM_CHARCONV)
      s.flags |= M_CHARCONV;
    if (flags & M_CM_REPLYING)
      s.flags |= M_REPLYING;

    if (WithCrypto && flags & M_CM_VERIFY)
      s.flags |= M_VERIFY;

    rc = mutt_body_handler (body, &s);
  }
  else if (WithCrypto
           && (flags & M_CM_DECODE_CRYPT) && (hdr->security & ENCRYPT)) {
    BODY *cur;
    FILE *fp;

    if ((WithCrypto & APPLICATION_PGP)
        && (flags & M_CM_DECODE_PGP) && (hdr->security & APPLICATION_PGP) &&
        hdr->content->type == TYPEMULTIPART) {
      if (crypt_pgp_decrypt_mime (fpin, &fp, hdr->content, &cur))
        return (-1);
      fputs ("MIME-Version: 1.0\n", fpout);
    }

    if ((WithCrypto & APPLICATION_SMIME)
        && (flags & M_CM_DECODE_SMIME) && (hdr->security & APPLICATION_SMIME)
        && hdr->content->type == TYPEAPPLICATION) {
      if (crypt_smime_decrypt_mime (fpin, &fp, hdr->content, &cur))
        return (-1);
    }

    mutt_write_mime_header (cur, fpout);
    fputc ('\n', fpout);

    fseeko (fp, cur->offset, 0);
    if (mutt_copy_bytes (fp, fpout, cur->length) == -1) {
      fclose (fp);
      mutt_free_body (&cur);
      return (-1);
    }
    mutt_free_body (&cur);
    fclose (fp);
  }
  else {
    fseeko (fpin, body->offset, 0);
    if (flags & M_CM_PREFIX) {
      int c;
      size_t bytes = body->length;

      fputs (prefix, fpout);

      while ((c = fgetc (fpin)) != EOF && bytes--) {
        fputc (c, fpout);
        if (c == '\n') {
          fputs (prefix, fpout);
        }
      }
    }
    else if (mutt_copy_bytes (fpin, fpout, body->length) == -1)
      return -1;
  }

  if ((flags & M_CM_UPDATE) && (flags & M_CM_NOHEADER) == 0
      && new_offset != -1) {
    body->offset = new_offset;
    mutt_free_body (&body->parts);
  }

  return rc;
}

int
mutt_copy_message (FILE * fpout, CONTEXT * src, HEADER * hdr, int flags,
                   int chflags)
{
  MESSAGE *msg;
  int r;

  if ((msg = mx_open_message (src, hdr->msgno)) == NULL)
    return -1;
  if ((r =
       _mutt_copy_message (fpout, msg->fp, hdr, hdr->content, flags,
                           chflags)) == 0 && (ferror (fpout)
                                              || feof (fpout))) {
    debug_print (1, ("_mutt_copy_message failed to detect EOF!\n"));
    r = -1;
  }
  mx_close_message (&msg);
  return r;
}

/* appends a copy of the given message to a mailbox
 *
 * dest		destination mailbox
 * fpin		where to get input
 * src		source mailbox
 * hdr		message being copied
 * body		structure of message being copied
 * flags	mutt_copy_message() flags
 * chflags	mutt_copy_header() flags
 */

int
_mutt_append_message (CONTEXT * dest, FILE * fpin, CONTEXT * src,
                      HEADER * hdr, BODY * body, int flags, int chflags) {
  char buf[STRING];
  MESSAGE *msg;
  int r;

  fseeko(fpin, hdr->offset, 0);
  if (fgets (buf, sizeof (buf), fpin) == NULL)
    return (-1);
  if ((msg = mx_open_new_message (dest, hdr, is_from (buf, NULL, 0, NULL) ? 0 : M_ADD_FROM)) == NULL)
    return (-1);
  if (dest->magic == M_MBOX || dest->magic == M_MMDF)
    chflags |= CH_FROM | CH_FORCE_FROM;
  chflags |= (dest->magic == M_MAILDIR ? CH_NOSTATUS : CH_UPDATE);
  r = _mutt_copy_message (msg->fp, fpin, hdr, body, flags, chflags);
  if (mx_commit_message (msg, dest) != 0)
    r = -1;

  mx_close_message (&msg);
  return r;
}

int
mutt_append_message (CONTEXT * dest, CONTEXT * src, HEADER * hdr, int cmflags,
                     int chflags)
{
  MESSAGE *msg;
  int r;

  if ((msg = mx_open_message (src, hdr->msgno)) == NULL)
    return -1;
  r =
    _mutt_append_message (dest, msg->fp, src, hdr, hdr->content, cmflags,
                          chflags);
  mx_close_message (&msg);
  return r;
}

/*
 * This function copies a message body, while deleting _in_the_copy_
 * any attachments which are marked for deletion.
 * Nothing is changed in the original message -- this is left to the caller.
 *
 * The function will return 0 on success and -1 on failure.
 */
static int copy_delete_attach (BODY * b, FILE * fpin, FILE * fpout,
                               char *date)
{
  BODY *part;

  for (part = b->parts; part; part = part->next) {
    if (part->deleted || part->parts) {
      /* Copy till start of this part */
      if (mutt_copy_bytes (fpin, fpout, part->hdr_offset - ftello (fpin)))
        return -1;

      if (part->deleted) {
        fprintf (fpout,
                 "Content-Type: message/external-body; access-type=x-mutt-deleted;\n"
                 "\texpiration=%s; length=" OFF_T_FMT "\n"
                 "\n", date + 5, part->length);
        if (ferror (fpout))
          return -1;

        /* Copy the original mime headers */
        if (mutt_copy_bytes (fpin, fpout, part->offset - ftello (fpin)))
          return -1;

        /* Skip the deleted body */
        fseeko (fpin, part->offset + part->length, SEEK_SET);
      }
      else {
        if (copy_delete_attach (part, fpin, fpout, date))
          return -1;
      }
    }
  }

  /* Copy the last parts */
  if (mutt_copy_bytes (fpin, fpout, b->offset + b->length - ftello (fpin)))
    return -1;

  return 0;
}

/* 
 * This function is the equivalent of mutt_write_address_list(),
 * but writes to a buffer instead of writing to a stream.
 * mutt_write_address_list could be re-used if we wouldn't store
 * all the decoded headers in a huge array, first. 
 *
 * XXX - fix that. 
 */

static void format_address_header (char **h, ADDRESS * a)
{
  char buf[HUGE_STRING];
  char cbuf[STRING];
  char c2buf[STRING];

  int l, linelen, buflen, count;

  linelen = str_len (*h);
  buflen = linelen + 3;


  mem_realloc (h, buflen);
  for (count = 0; a; a = a->next, count++) {
    ADDRESS *tmp = a->next;

    a->next = NULL;
    *buf = *cbuf = *c2buf = '\0';
    rfc822_write_address (buf, sizeof (buf), a, 0);
    a->next = tmp;

    l = str_len (buf);
    if (count && linelen + l > 74) {
      strcpy (cbuf, "\n\t");    /* __STRCPY_CHECKED__ */
      linelen = l + 8;
    }
    else {
      if (a->mailbox) {
        strcpy (cbuf, " ");     /* __STRCPY_CHECKED__ */
        linelen++;
      }
      linelen += l;
    }
    if (!a->group && a->next && a->next->mailbox) {
      linelen++;
      buflen++;
      strcpy (c2buf, ",");      /* __STRCPY_CHECKED__ */
    }

    buflen += l + str_len (cbuf) + str_len (c2buf);
    mem_realloc (h, buflen);
    strcat (*h, cbuf);          /* __STRCAT_CHECKED__ */
    strcat (*h, buf);           /* __STRCAT_CHECKED__ */
    strcat (*h, c2buf);         /* __STRCAT_CHECKED__ */
  }

  /* Space for this was allocated in the beginning of this function. */
  strcat (*h, "\n");            /* __STRCAT_CHECKED__ */
}

static int address_header_decode (char **h)
{
  char *s = *h;
  int l;

  ADDRESS *a = NULL;

  switch (tolower ((unsigned char) *s)) {
  case 'r':
    {
      if (ascii_strncasecmp (s, "return-path:", 12) == 0) {
        l = 12;
        break;
      }
      else if (ascii_strncasecmp (s, "reply-to:", 9) == 0) {
        l = 9;
        break;
      }
      return 0;
    }
  case 'f':
    {
      if (ascii_strncasecmp (s, "from:", 5))
        return 0;
      l = 5;
      break;
    }
  case 'c':
    {
      if (ascii_strncasecmp (s, "cc:", 3))
        return 0;
      l = 3;
      break;

    }
  case 'b':
    {
      if (ascii_strncasecmp (s, "bcc:", 4))
        return 0;
      l = 4;
      break;
    }
  case 's':
    {
      if (ascii_strncasecmp (s, "sender:", 7))
        return 0;
      l = 7;
      break;
    }
  case 't':
    {
      if (ascii_strncasecmp (s, "to:", 3))
        return 0;
      l = 3;
      break;
    }
  case 'm':
    {
      if (ascii_strncasecmp (s, "mail-followup-to:", 17))
        return 0;
      l = 17;
      break;
    }
  default:
    return 0;
  }

  if ((a = rfc822_parse_adrlist (a, s + l)) == NULL)
    return 0;

  mutt_addrlist_to_local (a);
  rfc2047_decode_adrlist (a);

  *h = mem_calloc (1, l + 2);

  strfcpy (*h, s, l + 1);

  format_address_header (h, a);

  rfc822_free_address (&a);

  mem_free (&s);
  return 1;
}
