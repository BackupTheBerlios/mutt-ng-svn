/** @ingroup muttng_conf */
/**
 * @file muttng/config/set_command.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: @c set Command handler
 */
#include <iostream>

#include "core/buffer.h"
#include "core/str.h"
#include "core/intl.h"
#include "core/hash.h"

/** all storage for variables is static here */
#define SET_COMMAND_CPP 1
#include "global_variables.h"

#include "abstract_option.h"
#include "set_command.h"
#include "string_option.h"
#include "num_option.h"
#include "bool_option.h"
#include "option.h"

/** shortcut */
#define UL      unsigned long

/** All configuration options we know */
static option_t Options[] = {
  /* START options Variable Reference */

  { SetCommand::T_BOOL,         "allow_8bit",           "yes",          OPT_ALLOW8BIT   },
  /*
  ** <p>
  ** Controls whether 8-bit data is converted to 7-bit using either
  ** <tt>quoted-printable</tt> or <tt>base64</tt> encoding when sending mail.
  ** </p>
  */

  { SetCommand::T_NUM,          "debug_level",          "0",            (UL) &DebugLevel     },
  /*
  ** <p>
  ** This variable specifies the current debug level and, currently,
  ** must be in the range 0 to 5. The value 0 has the special meaning
  ** that no debug is to be generated. From a value of 1 to 5 the
  ** amount of debug info written increases drastically.
  ** </p>
  ** <p>
  ** Debug files will be written to the home directory by default and to
  ** the current if the home directory cannot be determinded.
  ** </p>
  ** <p>
  ** Debug files will have a name of the following format:
  ** <tt>.[scope].[pid].[id].log</tt>, whereby:
  ** <ul>
  **   <li><tt>[scope]</tt> is an identifier for where the output came.
  **     One file will be created for <tt>libmuttng</tt> and one for the
  **     tool using it such as <man>muttng</man> or <man>muttgn-conf</man>
  **     </li>
  **   <li><tt>[pid]</tt> is the current process ID</li>
  **   <li><tt>[id]</tt> is the debug sequence number. For the first debug
  **     file of the current session it'll be 1 and increased for
  **     subsequent enabling/disabling of debug output via this variable</li>
  ** </ul>
  ** </p>
  */

  { SetCommand::T_STRING,       "assumed_charset",      "us-ascii",     (UL) &AssumedCharset    },

  { SetCommand::T_NUM,          "umask",                "0077",         (UL) &Umask     },
  /*
  ** <p>
  ** This variable specifies the <em>octal</em> permissions for
  ** <man sect="2">umask</man>. See <man>chmod</man> for possible
  ** value.
  ** </p>
  */

  /* END */
  { 0,                          NULL,   NULL,   0 }
};

static void* OptHash = NULL;

SetCommand::SetCommand (void) : AbstractCommand () {}
SetCommand::~SetCommand (void) {
  hash_del (&OptHash, NULL);
}

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

  OptHash = hash_new ((sizeof (Options) / sizeof (option_t))*2, 0);

  for (i = 0; Options[i].name; i++) {

    hash_add (OptHash, Options[i].name, (HASH_ITEMTYPE) &Options[i]);

    if (str_eq2 (Options[i].name, "debug_level", 11) &&
        DebugLevel != 0)
      continue;

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
