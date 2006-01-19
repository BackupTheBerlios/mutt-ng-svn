/**
 * @file core/core.c
 * @brief Implementation: Misc library functions
 *
 * This file is published under the GNU General Public License.
 */
#include "core.h"

#include "sigs.h"
#include "intl.h"
#include "conv.h"

static unsigned short init = 0;

int core_init(void) {
  if (init) return 1;
  sigs_signal_init();
#ifdef CORE_INTL
  intl_init();
#endif
  conv_init();
  init = 1;
  return 1;
}

int core_cleanup(void) {
  conv_cleanup();
  return 1;
}
