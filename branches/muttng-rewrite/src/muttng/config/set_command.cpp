/**
 * @file muttng/config/set_command.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief @c set Command handler implementation
 */
#include "core/buffer.h"
#include "core/str.h"

/** all storage for variables is static here */
#define SET_COMMAND_CPP 1
#include "global_variables.h"

#include "set_command.h"
#include "string_option.h"
#include "option.h"

SetCommand::SetCommand () {}
SetCommand::~SetCommand () {}

#define UL      unsigned long

/** All configuration options we know */
static option_t Options[] = {
  { SetCommand::T_STRING,       "foo",  "Foo",  (UL) &Foo },
  /* last */
  { 0,                          NULL,   NULL,   0 }
};

bool SetCommand::handle (buffer_t* line, buffer_t* error, unsigned long data) {
  SetCommand::commands command = (SetCommand::commands) data;
  (void) line;
  (void) error;
  (void) command;
  return (true);
}

void SetCommand::init (void) {
  int i = 0;
  buffer_t tmp;

  this->handlers[T_STRING] = new StringOption ();

  /* dump Options[] into hash table */
  for (i = 0; Options[i].name; i++) {
    buffer_init ((&tmp));
    tmp.str = (char*) Options[i].init;
    tmp.len = str_len (tmp.str);
    tmp.size = tmp.len;
    this->handlers[Options[i].type]->fromString (&tmp, &Options[i]);
  }
}

bool SetCommand::print (buffer_t* dst, bool changedOnly, bool annotated) {
  int i = 0, eq = 0;
  buffer_t tmp;

  for (i = 0; Options[i].name; i++) {
    buffer_init ((&tmp));
    if (!this->handlers[Options[i].type]->toString (&Options[i], &tmp)) {
      buffer_free (&tmp);
      return (false);
    }
    eq = str_eq (tmp.str, Options[i].init);
    if (changedOnly && eq) {
      buffer_free (&tmp);
      continue;
    }
    buffer_add_str (dst, Options[i].name, -1);
    buffer_add_str (dst, " = \"", 4);
    buffer_add_str (dst, tmp.str, tmp.len);
    buffer_add_str (dst, "\"", 1);
    if (annotated && !eq) {
      buffer_add_str (dst, " (default: \"", 12);
      buffer_add_str (dst, Options[i].init, -1);
      buffer_add_str (dst, "\")", 2);
    }
    if (Options[i+1].name)
      buffer_add_ch (dst, '\n');
    buffer_free ((&tmp));
  }
  return (true);
}
