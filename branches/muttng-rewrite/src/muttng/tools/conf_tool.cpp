/** @ingroup muttng_bin_conf */
/**
 * @file muttng/tools/conf_tool.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief muttng(1) implementation
 */
#include <unistd.h>

#include <iostream>

#include "core/intl.h"

#include "ui/ui_plain.h"

#include "conf_tool.h"

using namespace std;

/** Usage string for @c muttng(1). */
static const char* Usage = N_("\
Usage: muttng-conf [-d] [-a]\n\
  ");

/** Help string for @c muttng(1). */
static const char* Options = N_("\
Options:\n\
  -a\t\tAnnotated: print defaults if value differs\n\
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

int ConfTool::main (void) {
  int ch = 0, rc = 0;
  bool changedOnly = false, annotated = false;

  while ((ch = getopt (this->argc, this->argv, "aD" GENERIC_ARGS)) != -1) {
    switch (ch) {
      case 'a': annotated = true; break;
      case 'D': changedOnly = true; break;
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
  rc = this->config->print (this->ui->getConfigScreen (),
                            changedOnly, annotated);
  this->end ();
  return (rc ? 0 : 1);
}

const char* ConfTool::getName (void) {
  return ("muttng-conf");
}
