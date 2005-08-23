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

/*
 * body handler implementing RfC 3676 for format=flowed
 */

int rfc3676_handler (BODY * a, STATE * s);

#endif /* !_MUTT_RFC3676_H */