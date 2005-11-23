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
#include "quad_option.h"
#include "url_option.h"
#include "option.h"

/** shortcut */
#define UL      unsigned long

/** All configuration options we know */
static option_t Options[] = {
  /* START */

  { SetCommand::T_QUAD,         "abort_unmodified",     "yes",          Q_ABORT,        0 },
  /*
  ** If set to <val>yes</val>, composition will automatically abort after
  ** editing the message body if no changes are made to the file (this
  ** check only happens after the <em>first</em> edit of the file).
  ** When set to <val>no</val>, composition will never be aborted.
  */

  { SetCommand::T_BOOL,         "allow_8bit",           "yes",          OPT_ALLOW8BIT,  0 },
  /*
  ** <p>
  ** Controls whether 8-bit data is converted to 7-bit using either
  ** <enc>quoted-printable</enc> or <enc>base64</enc> encoding when
  ** sending mail.
  ** </p>
  */

  { SetCommand::T_STRING,       "assumed_charset",      "us-ascii",     (UL) &AssumedCharset,   0 },
  /*
  ** <p>
  ** This variable is a colon-separated list of character encoding
  ** schemes for messages without character encoding indication.
  ** Header field values and message body content without character encoding
  ** indication would be assumed that they are written in one of this list.
  ** </p>
  ** <p>
  ** By default, all the header fields and message body without any charset
  ** indication are assumed to be in <enc>us-ascii</enc>.
  ** </p>
  ** <p>
  ** For example, Japanese users might prefer this:
  ** <pre>
  ** set assumed_charset="iso-2022-jp:euc-jp:shift_jis:utf-8"
  ** </pre>
  ** </p>
  ** <p>
  ** However, only the first content is valid for the message body.
  ** This variable is valid only if <varref>strict_mime</varref> is <val>unset</val>.
  ** </p>
  */

  { SetCommand::T_NUM,          "debug_level",          "0",            (UL) &DebugLevel,       1},
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

  { SetCommand::T_URL,          "folder",               "file:///tmp/Mail",    (UL) Maildir,      1 },
  /*
  ** <p>
  ** Specifies the default location of your mailboxes.
  ** </p>
  ** <p>
  ** A <tt>+</tt> or <tt>=</tt> at the beginning of a pathname will be expanded
  ** to the value of this variable.
  ** </p>
  ** <p>
  ** Note that if you change this variable from
  ** the default value you need to make sure that the assignment occurs
  ** <em>before</em> you use <tt>+</tt> or <tt>=</tt> for any other variables
  ** since expansion takes place during the <cmdref>set</cmdref> command.
  ** </p>
  */

  { SetCommand::T_URL,          "mbox",                 "file:///tmp/mbox",    (UL) Mbox,      0 },
  /*
  ** <p>
  ** This specifies the folder into which read mail in your
  ** <varref>spoolfile</varref> folder will be appended.
  ** </p>
  */

  { SetCommand::T_URL,          "postponed",            "file:///tmp/postponed",    (UL) Postponed,      1 },
  /*
  ** <p>
  ** Mutt-ng allows you to indefinitely ``<funcref>postpone</funcref> sending a message'' which
  ** you are editing. When you choose to postpone a message, Mutt-ng saves it
  ** in the mailbox specified by this variable.
  ** </p>
  ** <p>
  ** Also see the <varref>postpone</varref> variable.
  ** </p>
  */

  { SetCommand::T_URL,          "record",               "",                      (UL) Outbox,      0 },
  /*
  ** <p>
  ** This specifies the file into which your outgoing messages should be
  ** appended. (This is meant as the primary method for saving a copy of
  ** your messages, but another way to do this is using the <cmdref>my_hdr</cmdref>
  ** command to create a <hdr>Bcc:</hdr> header field with your email address in it.)
  ** </p>
  ** <p>
  ** The value of <varref>record</varref> is overridden by the <varref>force_name</varref>
  ** and <varref>save_name</varref> variables, and the <cmdref>fcc-hook</cmdref> command.
  ** </p>
  */

  { SetCommand::T_NUM,          "umask",                "0077",         (UL) &Umask,    0 },
  /*
  ** <p>
  ** This variable specifies the <em>octal</em> permissions for
  ** <man sect="2">umask</man>. See <man>chmod</man> for possible
  ** value.
  ** </p>
  */

  /* END */
  { 0,                          NULL,   NULL,   0,      0 }
};

/** for faster access: hash table of options we know */
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
  buffer_t tmp, error, error2;
  AbstractCommand::state state = AbstractCommand::S_OK_UNCHANGED;

  (void) Homedir;

  BoolOptions = (int*) array_bit_alloc (OPT_LAST);
  QuadOptions = (int*) array_bit_alloc ((Q_LAST * 2 + 7) / 8);

  this->handlers[T_STRING] = new StringOption ();
  this->handlers[T_BOOL] = new BoolOption ();
  this->handlers[T_NUM] = new NumOption ();
  this->handlers[T_QUAD] = new QuadOption ();
  this->handlers[T_URL] = new URLOption ();

  this->types[T_STRING] = _("string");
  this->types[T_BOOL] = _("boolean");
  this->types[T_NUM] = _("number");
  this->types[T_QUAD] = _("quad-option");
  this->types[T_URL] = _("url");

  OptHash = hash_new ((sizeof (Options) / sizeof (option_t))*2, 0);

  buffer_init ((&error2));

  for (i = 0; Options[i].name; i++) {

    bool isDebug = str_eq2 (Options[i].name, "debug_level", 11);

    hash_add (OptHash, Options[i].name, (HASH_ITEMTYPE) &Options[i]);

    if (isDebug && DebugLevel != 0)
      continue;

    buffer_init ((&tmp));
    tmp.str = (char*) Options[i].init;
    tmp.len = str_len (tmp.str);
    tmp.size = tmp.len;
    buffer_shrink (&error2, 0);
    state = this->handlers[Options[i].type]->fromString (AbstractOption::T_SET,
                                                         &tmp, &Options[i],
                                                         &error2);
    switch (state) {
      case AbstractCommand::S_OK_CHANGED:
        if (Options[i].emit)
          switch (Options[i].type) {
          case T_NUM:
            event->sigNumOptChange.emit (event->getContext (), Options[i].name,
                                         *((int*) Options[i].data));
            break;
          case T_URL:
            event->sigURLOptChange.emit (event->getContext (), Options[i].name,
                                         (url_t*) Options[i].data);
          default:
            break;
          }
      /* fall through */
      case AbstractCommand::S_OK_UNCHANGED:
        break;
      case AbstractCommand::S_VALUE:
      case AbstractCommand::S_CMD:
        buffer_init ((&error));
        buffer_add_str (&error, _("Error for option $"), -1);
        buffer_add_str (&error, Options[i].name, -1);
        if (state == AbstractCommand::S_VALUE) {
          buffer_add_str (&error, _(": invalid default value: \""), -1);
          buffer_add_buffer (&error, &tmp);
          buffer_add_ch (&error, '"');
          if (error2.len) {
            buffer_add_str (&error, ": ", 2);
            buffer_add_buffer (&error, &error2);
          }
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
