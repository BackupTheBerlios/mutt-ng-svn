/** @ingroup muttng_conf */
/**
 * @file muttng/config/global_variables.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: UI internal configuration
 *
 * This file is published under the GNU General Public License.
 */
#ifndef MUTTNG_CONFIG_GLOBAL_VARIABLES_H
#define MUTTNG_CONFIG_GLOBAL_VARIABLES_H

#include "core/array.h"
#include "libmuttng/util/url.h"

#ifdef WHERE
#undef WHERE
#endif

#ifdef INITVAL
#undef INITVAL
#endif

#ifdef MUTTNG_MAIN_CPP
/** modifier for actual storage of variables */
#define WHERE
/** set initial value for static and none for extern */
#define INITVAL(X)      =X
#else
/** modifier for actual storage of variables */
#define WHERE extern
/** set initial value for static and none for extern */
#define INITVAL(X)
#endif

/** storage for @ref option_config_charset */
WHERE char* ConfigCharset INITVAL(NULL);

#endif /* !MUTTNG_CONFIG_GLOBAL_VARIABLES_H */
