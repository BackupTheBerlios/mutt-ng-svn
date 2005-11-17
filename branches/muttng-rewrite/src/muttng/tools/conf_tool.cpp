/** @ingroup muttng_bin_conf */
/**
 * @file muttng/tools/conf_tool.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: muttng(1)
 */
#include <unistd.h>

#include <iostream>

#include "core/intl.h"
#include "core/mem.h"
#include "core/str.h"

#include "ui/ui_plain.h"

#include "conf_tool.h"

using namespace std;

/** Usage string for @c muttng(1). */
static const char* Usage = N_("\
Usage: muttng-conf [-D] [-a]\n\
       muttng-conf -B [-a]\n\
  ");

/** Help string for @c muttng(1). */
static const char* Options = N_("\
Options:\n\
  -a\t\tAnnotated: print defaults if value differs\n\
  -B\t\tList all bindings\n\
  -D\t\tDump all non-default variable values\n\
  ");

ConfTool::ConfTool (int argc, char** argv) : Tool (argc, argv) {
  this->ui = new UIPlain ();
}

ConfTool::~ConfTool () {}

void ConfTool::getUsage (buffer_t* dst) {
  buffer_add_str (dst, Usage, -1);
  buffer_add_ch (dst, '\n');
  buffer_add_str (dst, Options, -1);
}

void ConfTool::do_opts (bool annotated, bool changed) {
  buffer_t name, type, init, value;
  int idx = 0;

  buffer_init ((&name));
  buffer_init ((&type));
  buffer_init ((&init));
  buffer_init ((&value));

  idx = 0;
  while (config->getSingleOption (&idx, &name, &type, &init, &value)) {
    int eq = buffer_equal2 (&init, &value);
    if (changed && eq)
      continue;
    cout << name.str << " = \"" << value.str << "\"";
    if (annotated) {
      cout << " #";
      if (!eq)
        cout << " (default: \"" << init.str << "\")";
      cout << " (type: " << type.str << ")";
    }
    cout << endl;
  }

  buffer_free (&name);
  buffer_free (&type);
  buffer_free (&init);
  buffer_free (&value);

}

void ConfTool::do_bind (bool annotated, bool changed) {
  int i = 0, j = 0, k = 0;
  const binding_t* descr = NULL;
  std::vector<Event::group>* groups = NULL;
  std::vector<Event::event>* events = NULL;

  for (i = 0; i < (int) Event::C_LAST; i++) {
    /* first: continue if no groups within context */
    if (!(groups = event->getGroups ((Event::context) i)))
      continue;
    for (j = 0; j < (int) groups->size (); j++) {

      /* ignore internal groups */
      if (groups->at (j) == Event::G_INTERNAL)
        continue;

      /* for each group, print context and group name */
      if (annotated)
        cout << _("# Context: ") << event->getContextName ((Event::context) i) <<
                _(", group: ") << event->getGroupName (groups->at (j)) << endl;
      /* now try to get all events so we can sort functions by group */
      if (!(events = event->getEvents ((Event::context) i, groups->at (j))))
        continue;

      for (k = 0; k < (int) events->size (); k++) {
        descr = event->getHelp ((Event::context) i, events->at (k));
        /* only functions have a help text, read: ignore internal */
        if (!descr->help || (changed && str_eq (descr->key, descr->defkey)))
          continue;
        cout << "bind " << event->getContextName ((Event::context) i) << " " 
             << descr->key << " '<" << descr->name << ">'";
        if (annotated)
          cout << " # " << descr->help;
        cout << endl;
      }
      delete (events);
    }
    if (annotated)
      cout << endl;
    delete (groups);
  }
}

int ConfTool::main (void) {
  int ch = 0, rc = 0;
  bool changedOnly = false, annotated = false;
  modes mode = M_OPTS;

  while ((ch = getopt (this->argc, this->argv, "aBD" GENERIC_ARGS)) != -1) {
    switch (ch) {
      case 'a': annotated = true; break;
      case 'D': changedOnly = true; break;
      case 'B': mode = M_BIND; break;
      default:
        rc = genericArg (ch, optarg);
        if (rc == -1)
          displayUsage ();
        if (rc != 1)
          return (rc == 0);
        break;
    }
  }

  if (!this->start ())
    return (1);

  switch (mode) {
    case M_OPTS: do_opts (annotated, changedOnly); break;
    case M_BIND: do_bind (annotated, changedOnly); break;
  }

  this->end ();
  return (rc ? 0 : 1);
}

const char* ConfTool::getName (void) {
  return ("muttng-conf");
}
