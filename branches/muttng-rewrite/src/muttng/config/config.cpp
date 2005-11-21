/* doxygen documentation {{{ */
/**
 * @page page_incompat_config Incompatible Changes: Configuration
 *
 * Compared to mutt and mutt-ng, there're some incompatible changes:
 *
 *   - The <code>set ?foo</code> syntax isn't allowed any longer. For
 *     this purpose, the @c query command is to be used instead:
 *     <code>query foo</code>
 *   - The <code>set <b>no</b>foo</code> syntax isn't allowed any
 *     longer. For this purpose, use <code>unset foo</code> instead.
 *   - The <code>set <b>inv</b>foo</code> syntax isn't allowed any
 *     longer. For this purpose, use <code>toggle foo</code> instead.
 *   - The <code>set <b>&</b>foo</code> syntax isn't allowed any
 *     longer. For this purpose, use <code>reset foo</code> instead.
 */
/**
 * @ingroup muttng
 * @addtogroup muttng_conf Configuration
 * @{
 *
 *   @section muttng_config_types Option types
 *
 *     The following types of options are known:
 *
 *       - @c bool: A boolean option. Possible values are: @c yes, @c no,
 *         @c true, @c false, @c 1, @c 0.
 *       - @c number: A numeric option. Most of the options are
 *         positive-only numbers but some allow negative values. The
 *         documentation for the options list these and, in addition,
 *         mutt-ng will perform strict checks and report error for
 *         invalid ranges.
 *       - @c string: A string. As for numbers, most options allow any
 *         value but some allow only for a few magic words given in the
 *         option documentation. Mutt-ng will perform strict checks and
 *         print errors for invalid values, i.e. non-magic words.
 *       - @c quad-option. A "question" with the feature to already
 *         answer it and thus skip any prompts with a given value.
 *         Possible values are: @c yes, @c no, @c ask-yes and @c ask-no.
 *         When an option is used and it's either @c ask-yes and @c ask-no,
 *         a prompt will query for a result highlighting the given
 *         choice, @c yes or @c no respectively. With settings of @c yes
 *         or @c no, the question is assumed to be answered already.
 *
 *   @section muttng_config_commands Commands
 *
 *     The following commands are available for dealing with options:
 *     @c set, @c unset, @c reset, @c toggle and @c query.
 *
 *     The @c set command sets an option to a value. If no particular
 *     value is given for @c quad-option and @c bool types, @c yes is
 *     assumed.
 *
 *     The @c unset command unsets a value to a neutral value. The
 *     neutral values depend on the option's type:
 *
 *       - @c bool and @c quad-option: @c no
 *       - @c number: @c 0
 *       - @c string: the empty string ""
 *
 *     The @c reset command changes a value to its default.
 *
 *     The @c toggle command inverts a @c bool or @c quad-option value
 *     and is not allowed for other option types.
 *
 *     The @c query command displays the current value for any option.
 *
 *   @section muttng_config_doc Documentation
 */
/* }}} */
/**
 * @file muttng/config/config.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: Configuration
 */
#include <sys/types.h>
#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>

#include "global_variables.h"

#include "core/buffer.h"
#include "core/str.h"

#include "set_command.h"
#include "config.h"

/** structure for known command */
typedef struct {
  /** 
   * Which type of command. This is used to find the proper handler in
   * the Config::handlers[] array.
   */
  Config::commands type;
  /** name of command */
  const char* name;
  /** Additional data passed down to handling class */
  unsigned long data;
} command_t;

/** Table of known commands */
static command_t Commands[] = {
  { Config::C_SET,      "set",          AbstractOption::T_SET },
  { Config::C_SET,      "unset",        AbstractOption::T_UNSET },
  { Config::C_SET,      "reset",        AbstractOption::T_RESET },
  { Config::C_SET,      "toggle",       AbstractOption::T_TOGGLE },
  { Config::C_SET,      "query",        AbstractOption::T_QUERY },
  /* last */
  { Config::C_INVALID,  NULL,           0 }
};

Config::Config (void) { (void) Commands; }
Config::~Config (void) {}

void Config::preinit (void) {
  char* p = NULL;

  /* setup misc. */
  if ((p = getenv ("HOME")) == NULL) {
    struct passwd* pw;
    if ((pw = getpwuid (getuid ())))
      p = pw->pw_dir;
  }
  if (p)
    Homedir = str_dup (p);
}

bool Config::init (UI* ui) {
  /* setup handlers and init */
  this->handlers[C_SET] = new SetCommand ();
  for (int i = 0; i < Config::C_INVALID; i++)
    if (!this->handlers[i]->init (ui))
      return (false);
  return (true);
}

bool Config::read (bool readGlobal, const char* file, buffer_t* error) {
  (void) file;
  (void) readGlobal;
  (void) error;
  return (true);
}

bool Config::getSingleOption (int* idx, buffer_t* name, buffer_t* type,
                              buffer_t* init, buffer_t* value) {
  if (name)
    buffer_shrink (name, 0);
  if (type)
    buffer_shrink (type, 0);
  if (init)
    buffer_shrink (init, 0);
  if (value)
    buffer_shrink (value, 0);
  return (this->handlers[C_SET]->getSingleOption (idx, name, type, init, value));
}
