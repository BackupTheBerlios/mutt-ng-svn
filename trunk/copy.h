/*
 * Copyright notice from original mutt:
 * Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */
#ifndef _MUTT_COPY_H
#define _MUTT_COPY_H

/* flags for mutt_copy_header() */
#define CH_UPDATE               (1<<0)  /* update the status and x-status fields? */
#define CH_WEED                 (1<<1)  /* weed the headers? */
#define CH_DECODE               (1<<2)  /* do RFC1522 decoding? */
#define CH_XMIT                 (1<<3)  /* transmitting this message? */
#define CH_FROM                 (1<<4)  /* retain the "From " message separator? */
#define CH_PREFIX               (1<<5)  /* use Prefix string? */
#define CH_NOSTATUS             (1<<6)  /* supress the status and x-status fields */
#define CH_REORDER              (1<<7)  /* Re-order output of headers */
#define CH_NONEWLINE            (1<<8)  /* don't output terminating newline */
#define CH_MIME                 (1<<9)  /* ignore MIME fields */
#define CH_UPDATE_LEN           (1<<10) /* update Lines: and Content-Length: */
#define CH_TXTPLAIN             (1<<11) /* generate text/plain MIME headers */
#define CH_NOLEN                (1<<12) /* don't write Content-Length: and Lines: */
#define CH_WEED_DELIVERED       (1<<13) /* weed eventual Delivered-To headers */
#define CH_FORCE_FROM           (1<<14) /* give CH_FROM precedence over CH_WEED? */
#define CH_NOQFROM              (1<<15) /* give CH_FROM precedence over CH_WEED? */
#define CH_UPDATE_IRT           (1<<16) /* update In-Reply-To: */
#define CH_UPDATE_REFS          (1<<17) /* update References: */

/* flags to _mutt_copy_message */
#define M_CM_NOHEADER           (1<<0)  /* don't copy the message header */
#define M_CM_PREFIX             (1<<1)  /* quote the message */
#define M_CM_DECODE             (1<<2)  /* decode the message body into text/plain */
#define M_CM_DISPLAY            (1<<3)  /* output is displayed to the user */
#define M_CM_UPDATE             (1<<4)  /* update structs on sync */
#define M_CM_WEED               (1<<5)  /* weed message/rfc822 attachment headers */
#define M_CM_CHARCONV           (1<<6)  /* perform character set conversions */
#define M_CM_PRINTING           (1<<7)  /* printing the message - display light */
#define M_CM_REPLYING           (1<<8)  /* replying the message */
#define M_CM_DECODE_PGP         (1<<9)  /* used for decoding PGP messages */
#define M_CM_DECODE_SMIME       (1<<10) /* used for decoding S/MIME messages */
#define M_CM_DECODE_CRYPT       (M_CM_DECODE_PGP | M_CM_DECODE_SMIME)
#define M_CM_VERIFY             (1<<11) /* do signature verification */

int mutt_copy_hdr (FILE *, FILE *, LOFF_T, LOFF_T, int, const char *);
int mutt_copy_header (FILE *, HEADER *, FILE *, int, const char *);
int _mutt_copy_message (FILE * fpout, FILE * fpin,
                        HEADER * hdr, BODY * body, int flags, int chflags);
int mutt_copy_message (FILE * fpout,
                       CONTEXT * src, HEADER * hdr, int flags, int chflags);
int _mutt_append_message (CONTEXT * dest, FILE * fpin, CONTEXT * src,
                          HEADER * hdr, BODY * body, int flags, int chflags);
int mutt_append_message (CONTEXT * dest, CONTEXT * src,
                         HEADER * hdr, int cmflags, int chflags);

#endif /* !_MUTT_COPY_H */
