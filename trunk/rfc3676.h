/*
 * Parts were written/modified by:
 * Andreas Krennmair <ak@synflood.at>
 * Peter J. Holzer <hjp@hjp.net>
 * Rocco Rutte <pdmef@cs.tu-berlin.de>
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */
#ifndef _MUTT_RFC3676_H
#define _MUTT_RFC3676_H

#include "mutt.h"
#include "state.h"

/* body handler implementing RfC 3676 for format=flowed */
int rfc3676_handler (BODY * a, STATE * s);

/*
 * this properly ensures correct quoting; correct is:
 * - no spaces within the complete quote prefix of line (sect. 4.5)
 * - change all quoting chars to '>' by force; see BUGS in srcdir
 */
void rfc3676_quote_line (STATE* s, char* dst, size_t dstlen,
                         const char* line);

#endif /* !_MUTT_RFC3676_H */
