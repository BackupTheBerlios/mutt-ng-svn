/**
 * @ingroup core
 */
/**
 * @file core/exit.h
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Interface: Fatal error handling
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
