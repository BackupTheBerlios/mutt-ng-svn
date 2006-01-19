/**
 * @file core/core.h
 * @brief Interface: Misc library functions
 *
 * This file is published under the GNU General Public License.
 */
#ifndef CORE_CORE_H
#define CORE_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize core library.
 * @return Success
 */
int core_init(void);

/**
 * Cleanup core library.
 * @return Success.
 */
int core_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif /* !CORE_CORE_H */
