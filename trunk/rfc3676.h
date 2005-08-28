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
 * this does the space-stuffing required as in 'MUST'
 * this is only used right after editing the initial message's content
 * as elsewhere it's too difficult to catch all circumstances right;
 * esp. with '>' which this routine doesn't cover... XXX
 */
void rfc3676_space_stuff (HEADER* hdr);

#endif /* !_MUTT_RFC3676_H */
