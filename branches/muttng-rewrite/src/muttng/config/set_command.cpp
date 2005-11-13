/**
 * @file muttng/config/set_command.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief @c set Command handler implementation
 */
#include <iostream>

#include "core/buffer.h"
#include "core/str.h"
#include "core/intl.h"

/** all storage for variables is static here */
#define SET_COMMAND_CPP 1
#include "global_variables.h"

#include "abstract_option.h"
#include "set_command.h"
#include "string_option.h"
#include "num_option.h"
#include "bool_option.h"
#include "option.h"

SetCommand::SetCommand () {}
SetCommand::~SetCommand () {}

/** shortcut */
#define UL      unsigned long

/** All configuration options we know */
static option_t Options[] = {
  { SetCommand::T_BOOL,         "allow_8bit",           "yes",          OPT_ALLOW8BIT   },
  { SetCommand::T_STRING,       "assumed_charset",      "us-ascii",     (UL) &AssumedCharset    },
  { SetCommand::T_NUM,          "umask",                "-1",           (UL) &Umask     },
  /* last */
  { 0,                          NULL,   NULL,   0 }
};

bool SetCommand::handle (buffer_t* line, buffer_t* error, unsigned long data) {
  (void) line;
  (void) error;
  (void) data;
  return (true);
}

bool SetCommand::init (UI* ui) {
  int i = 0;
  buffer_t tmp, error;
  AbstractOption::state state = AbstractOption::S_OK;

  (void) Homedir;

  BoolOptions = (int*) array_bit_alloc (OPT_LAST);

  this->handlers[T_STRING] = new StringOption ();
  this->handlers[T_BOOL] = new BoolOption ();
  this->handlers[T_NUM] = new NumOption ();

  this->types[T_STRING] = _("string");
  this->types[T_BOOL] = _("boolean");
  this->types[T_NUM] = _("number");

  /* dump Options[] into hash table */

  for (i = 0; Options[i].name; i++) {
    buffer_init ((&tmp));
    tmp.str = (char*) Options[i].init;
    tmp.len = str_len (tmp.str);
    tmp.size = tmp.len;
    state = this->handlers[Options[i].type]->fromString (AbstractOption::T_SET,
                                                         &tmp, &Options[i]);
    switch (state) {
      case AbstractOption::S_OK: break;
      case AbstractOption::S_VALUE:
      case AbstractOption::S_CMD:
        buffer_init ((&error));
        buffer_add_str (&error, _("Error for option $"), -1);
        buffer_add_str (&error, Options[i].name, -1);
        if (state == AbstractOption::S_VALUE) {
          buffer_add_str (&error, _(": invalid default value: \""), -1);
          buffer_add_buffer (&error, &tmp);
          buffer_add_ch (&error, '"');
        } else
          buffer_add_str (&error, _(": invalid command"), -1);
        buffer_add_str (&error, _(" (report this)"), -1);
        ui->displayError (error.str);
        buffer_free (&error);
        return (false);
    }
  }
  return (true);
}

bool SetCommand::getSingleOption (int* idx, buffer_t* name, buffer_t* type,
                                  buffer_t* init, buffer_t* value) {
  option_t* opt = NULL;
  if (*idx >= (int) (sizeof (Options) / sizeof (option_t) - 1))
    return (false);
  opt = &Options[*idx];
  if (name)
    buffer_add_str (name, opt->name, -1);
  if (type)
    buffer_add_str (type, types[opt->type], -1);
  if (init)
    buffer_add_str (init, opt->init, -1);
  if (value)
    handlers[opt->type]->toString (opt, value);
  (*idx)++;
  return (true);
}
