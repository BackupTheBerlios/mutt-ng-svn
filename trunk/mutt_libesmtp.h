/*
 * Copyright notice from original mutt:
 * [none]
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#ifndef _MUTT_LIBESMTP_H
#define _MUTT_LIBESMTP_H

#if defined (USE_LIBESMTP) && (defined (USE_SSL) || defined (USE_GNUTLS))
int mutt_libesmtp_check_usetls (const char*);
#endif

int mutt_libesmtp_invoke (ADDRESS * from,       /* the sender */
                          ADDRESS * to, ADDRESS * cc, ADDRESS * bcc,    /* recips */
                          const char *msg,      /* file containing message */
                          int eightbit);        /* message contains 8bit chars */

#endif /* !_MUTT_LIBESMTP_H */
