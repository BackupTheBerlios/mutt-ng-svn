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

#include "libmuttng/config/config_manager.h"

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
  this->ui = new UIPlain(getName());
}

ConfTool::~ConfTool () {}

void ConfTool::getUsage (buffer_t* dst) {
  buffer_add_str (dst, Usage, -1);
  buffer_add_ch (dst, '\n');
  buffer_add_str (dst, Options, -1);
}

void ConfTool::do_opts (bool annotated, bool changed) {
  (void)annotated;(void)changed;
  std::vector<const char*>* opts = ConfigManager::getAll();
  size_t i;
  buffer_t value, valid;
  const char* name, *init, *type;

  if (!opts)
    return;

  buffer_init(&value);
  buffer_init(&valid);

  for (i = 0; i < opts->size(); i++) {
    buffer_shrink(&value,0);
    buffer_shrink(&valid,0);
    Option* opt = ConfigManager::get(opts->at(i));
    name = opt->getName();
    type = opt->getType();
    init = opt->getInit();
    opt->query(&value);
    if (!opt->validity(&valid))
      buffer_shrink(&valid,0);
    int eq = buffer_equal1 (&value,init,-1);
    Option::prettyValue(&value);
    if (changed && eq)
      continue;
    cout << name << " = \"" << (NONULL(value.str)) << "\"";
    if (annotated) {
      cout << " #";
      if (!eq)
        cout << " (default: \"" << (NONULL(init)) << "\")";
      cout << " (type: " << (NONULL(type)) << ")";
      if (valid.len)
        cout << " (validity: " << valid.str << ")";
    }
    cout << endl;
  }

  delete opts;
  buffer_free (&value);
  buffer_free (&valid);
}

void ConfTool::do_bind (bool annotated, bool changed) {
  const binding_t* descr = NULL;
  std::vector<Event::group>* groups = NULL;
  std::vector<Event::event>* events = NULL;
  Event::context i = C_0;
  Event::group j = G_0;
  Event::event k = E_0;

  for (i = C_0; i < Event::C_LAST; i++) {
    /* first: continue if no groups within context */
    if (!(groups = event->getGroups (i)))
      continue;
    for (j = G_0; j < (int) groups->size (); j++) {

      /* ignore internal groups */
      if (groups->at (j) == Event::G_INTERNAL)
        continue;

      /* for each group, print context and group name */
      if (annotated)
        cout << _("# Context: ") << event->getContextName (i) <<
                _(", group: ") << event->getGroupName (groups->at (j)) << endl;
      /* now try to get all events so we can sort functions by group */
      if (!(events = event->getEvents (i, groups->at (j))))
        continue;

      for (k = E_0; k < (int) events->size (); k++) {
        descr = event->getHelp (i, events->at (k));
        /* only functions have a help text, read: ignore internal */
        if (!descr->help || (changed && str_eq (descr->key, descr->defkey)))
          continue;
        cout << "bind " << event->getContextName (i) << " " 
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
