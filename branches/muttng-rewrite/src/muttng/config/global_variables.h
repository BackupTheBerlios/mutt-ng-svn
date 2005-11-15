/** @ingroup muttng_conf */
/**
 * @file muttng/config/global_variables.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: UI internal configuration
 */
#ifndef MUTTNG_CONFIG_GLOBAL_VARIABLES_H
#define MUTTNG_CONFIG_GLOBAL_VARIABLES_H

#include "core/array.h"

#if SET_COMMAND_CPP
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

/** $HOME */
WHERE const char* Homedir INITVAL(NULL);

/** array for all boolean options */
WHERE int* BoolOptions INITVAL(NULL);

/** Set boolean option. @sa array_bit_set(). */
#define set_option(x) array_bit_set(BoolOptions,x)
/** Unset boolean option. @sa array_bit_unset(). */
#define unset_option(x) array_bit_unset(BoolOptions,x)
/** Toggle boolean option. @sa array_bit_toggle(). */
#define toggle_option(x) array_bit_toggle(BoolOptions,x)
/** Query boolean option. @sa array_bit_isset(). */
#define option(x) array_bit_isset(BoolOptions,x)

/** $assumed_charset */
WHERE char* AssumedCharset INITVAL(NULL);
/** $debug_level */
WHERE int DebugLevel INITVAL(0);
/** $umask */
WHERE int Umask INITVAL(0);

/** all boolean options we know */
enum {
  /** $allow_8bit */
  OPT_ALLOW8BIT = 0,
  /** last */
  OPT_LAST
};

#endif /* !MUTTNG_CONFIG_GLOBAL_VARIABLES_H */
