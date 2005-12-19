/**
 * @file core/core.c
 * @brief Implementation: Misc library functions
 */
#include "core.h"

#include "sigs.h"
#include "intl.h"
#include "conv.h"

int core_init(void) {
  sigs_signal_init();
  intl_init();
  conv_init();
  return 1;
}

int core_cleanup(void) {
  conv_cleanup();
  return 1;
}
