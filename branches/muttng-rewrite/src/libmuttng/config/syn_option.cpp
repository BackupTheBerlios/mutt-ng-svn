/** @addtogroup libmuttng_config */
/**
 * @file libmuttng/config/syn_option.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: Synonym option
 *
 * This file is published under the GNU General Public License.
 */
#include "syn_option.h"
#include "config_manager.h"

#include "core/intl.h"

SynOption::SynOption(const char* name_, const char* real_) : Option(name_,NULL), real(NULL) {
  real = ConfigManager::get(real_);
}

SynOption::SynOption(const char* name_, Option* real_) : Option(name_,NULL), real(real_) {}

bool SynOption::set(const char* value, buffer_t* error) {
  if (!real) return false;
  return real->set(value,error);
}

bool SynOption::unset() {
  if (!real) return false;
  return real->unset();
}

bool SynOption::reset() {
  if (!real) return false;
  return real->reset();
}

bool SynOption::toggle() {
  if (!real) return false;
  return real->toggle();
}

bool SynOption::query(buffer_t* dst) {
  if (!real) return false;
  return real->query(dst);
}

const char* SynOption::getType() {
  return _("synonym");
}

bool SynOption::validity(buffer_t* dst) {
  if (!real || !dst) return false;
  buffer_add_str(dst,_("synonym for $"),-1);
  buffer_add_str(dst,real->getName(),-1);
  return true;
}
