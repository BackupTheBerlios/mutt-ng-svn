/*
 * Copyright notice from original mutt:
 * [none]
 *
 * This file is part of mutt-ng, see http://www.muttng.org/.
 * It's licensed under the GNU General Public License,
 * please see the file GPL in the top level source directory.
 */

#ifndef _CRYPTHASH_H
# define _CRYPTHASH_H


# include <sys/types.h>
# if HAVE_INTTYPES_H
#  include <inttypes.h>
# else
#  if HAVE_STDINT_H
#   include <stdint.h>
#  endif
# endif

/* POINTER defines a generic pointer type */
typedef unsigned char *POINTER;

#endif
