/**
 * @addtogroup core Muttng Core
 * @{
 * This is a more or less general purpose library in C. Some of the
 * routines is legacy we still having its root in mutt(-ng). It's
 * self-contained, i.e. doesn't have any dependency on libmuttng or
 * muttng itself. Maybe we switch to using other projects like libowfat
 * (or so) in the future.
 */
/**
 * @file core/version.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Muttng core version number
 */
#ifndef MUTTNG_CORE_VERSION_H
#define MUTTNG_CORE_VERSION_H

/**
 * Get version of muttng core.
 * As this may be excluded as a separate project sometime, we're ready for it now. ;-)
 */
#define CORE_VERSION    "muttng-core 0.0"

#endif /* !MUTTNG_CORE_VERSION_H */

/** @} */
