/**
 * @file muttng/config/config.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Configuration implementation
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
  { Config::C_SET,      "set",          SetCommand::T_SET },
  { Config::C_SET,      "unset",        SetCommand::T_UNSET },
  { Config::C_SET,      "reset",        SetCommand::T_RESET },
  { Config::C_SET,      "toggle",       SetCommand::T_TOGGLE },
  /* last */
  { Config::C_INVALID,  NULL,           0 }
};

Config::Config (void) { (void) Commands; }
Config::~Config (void) {}

void Config::init (void) {
  char* p = NULL;

  /* setup handlers and init */
  this->handlers[C_SET] = new SetCommand ();
  for (int i = 0; i < Config::C_INVALID; i++)
    this->handlers[i]->init ();
  /* setup misc. */
  if ((p = getenv ("HOME")) == NULL) {
    struct passwd* pw;
    if ((pw = getpwuid (getuid ())))
      p = pw->pw_dir;
  }
  if (p)
    Homedir = str_dup (p);
}

bool Config::read (bool readGlobal, const char* file, buffer_t* error) {
  (void) file;
  (void) readGlobal;
  (void) error;
  return (true);
}

bool Config::print (ConfigScreen* configScreen, bool changedOnly,
                    bool annotated) {
  int i = 0;
  for (i = 0; i < Config::C_INVALID; i++)
    if (!this->handlers[i]->print (configScreen, changedOnly, annotated))
      return (false);
  return (true);
}
