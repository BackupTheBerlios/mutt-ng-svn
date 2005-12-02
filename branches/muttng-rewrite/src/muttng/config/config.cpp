/* doxygen documentation {{{ */
/**
 * @ingroup muttng
 * @addtogroup muttng_conf Configuration
 * @{
 *
 * This implementation doesn't support a number old syntax constructs,
 * see @ref sect_config-incompat for details.
 *
 * For supported option types, see @ref sect_config-types.
 *
 * For supported config commands, see @ref sect_config-commands.
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

/** @} */
