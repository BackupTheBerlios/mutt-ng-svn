/*
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

/*
 * interface of mx_t implementation for IMAP
 */

#ifndef _IMAP_MX_H
#define _IMAP_MX_H

#include <sys/stat.h>

#include "mx.h"

int imap_is_magic (const char*, struct stat*);
mx_t* imap_reg_mx (void);

#endif /* !_IMAP_MX_H */
