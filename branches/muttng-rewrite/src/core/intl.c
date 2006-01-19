/** @ingroup core_intl */
/**
 * @file core/intl.c
 * @brief Implementation: Internationalization
 *
 * This file is published under the GNU General Public License.
 */
#include "intl.h"

#include <locale.h>

/** package name for localization */
#define CORE_PACKAGE    "muttng"

void intl_init() {
  setlocale(LC_ALL,"");
  bindtextdomain(CORE_PACKAGE,CORE_LOCALEDIR);
  textdomain(CORE_PACKAGE);
  setlocale(LC_CTYPE,"");
}

void intl_encoding(const char* charset) {
#ifdef CORE_HAVE_BIND_TEXTDOMAIN_CODESET
  bind_textdomain_codeset(CORE_PACKAGE,charset);
#endif
  (void) charset;
}
