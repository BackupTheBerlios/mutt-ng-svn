/**
 * @file core/core.c
 * @brief Implementation: Misc library functions
 */
#include "core.h"

#include "sigs.h"

int core_init(void) {
  sigs_signal_init();
  return 1;
}
