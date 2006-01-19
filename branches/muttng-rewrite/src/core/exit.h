/**
 * @ingroup core
 * @addtogroup core_legacy Legacy
 * @{
 * <b>NOTE: THIS IS TO BE REMOVED EVENTUALLY</b>.
 */
/**
 * @file core/exit.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: Fatal error handling
 *
 * This file is published under the GNU General Public License.
 */
#ifndef MUTTNG_CORE_EXIT_H
#define MUTTNG_CORE_EXIT_H

#ifdef __cplusplus
extern "C" {
#endif

void exit_fatal (const char*, const char*, int, const char*, int);

#ifdef __cplusplus
}
#endif

#endif /* !MUTTNG_CORE_EXIT_H */

/** @} */
