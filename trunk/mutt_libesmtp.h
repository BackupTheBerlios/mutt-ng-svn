/*
 * Copyright notice from original mutt:
 * [none]
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#if !defined(LIBESMTP_H)
#define LIBESMTP_H

int mutt_invoke_libesmtp (ADDRESS * from,       /* the sender */
                          ADDRESS * to, ADDRESS * cc, ADDRESS * bcc,    /* recips */
                          const char *msg,      /* file containing message */
                          int eightbit);        /* message contains 8bit chars */

#endif /* !defined(LIBESMTP_H) */
