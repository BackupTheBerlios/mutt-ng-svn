/**
 * @file core/sigs.h
 * @author Copyright (C) 1996-2000 Michael R. Elkins <me@mutt.org>
 * @brief Interface: Signal handling
 */
#ifndef CORE_SIGS_H
#define CORE_SIGS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "core_features.h"

#ifndef CORE_SIGTYPE
#define CORE_SIGTYPE    int
#endif

/* Attempt to catch "ordinary" signals and shut down gracefully. */
CORE_SIGTYPE sigs_exit_handler (int sig);

CORE_SIGTYPE sigs_child_handler (int sig);
CORE_SIGTYPE sigs_sighandler (int sig);

#ifdef USE_SLANG_CURSES
int sigs_intr_hook (void);
#endif /* USE_SLANG_CURSES */

void sigs_signal_init (void);

/* signals which are important to block while doing critical ops */
void sigs_block_signals (void);

/* restore the previous signal mask */
void sigs_unblock_signals (void);

void sigs_block_signals_system (void);

void sigs_unblock_signals_system (int docatch);

void sigs_allow_interrupt (int disposition);

#ifdef __cplusplus
}
#endif

#endif /* !CORE_SIGS_H */
