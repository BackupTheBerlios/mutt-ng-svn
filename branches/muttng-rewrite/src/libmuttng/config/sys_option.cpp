/** @addtogroup libmuttng_config */
/**
 * @file libmuttng/config/sys_option.cpp
 * @author rocco rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: system option
 *
 * This file is published under the GNU General Public License.
 */
#include "sys_option.h"
#include "config_manager.h"

#include "core/intl.h"

SysOption::SysOption(const char* name_,const char* init_) : Option(name_,init_) {}

bool SysOption::set(const char* value,buffer_t* error) {
  if (error) {
    buffer_add_ch(error,'\'');
    buffer_add_str(error,value,-1);
    buffer_add_str(error,_("' is invalid for $"),-1);
    buffer_add_str(error,name,-1);
    buffer_add_str(error,_(": variable is read-only"),-1);
  }
  return false;
}

bool SysOption::unset() { return false; }
bool SysOption::reset() { return false; }
bool SysOption::toggle() { return false; }
bool SysOption::query(buffer_t* dst) {
  if (!dst) return false;
  buffer_add_str(dst,init,-1);
  return true;
}
const char* SysOption::getType() { return _("system"); }
bool SysOption::validity(buffer_t* dst) {
  if (!dst) return true;
  buffer_add_str(dst,_("read-only"),-1);
  return true;
}
