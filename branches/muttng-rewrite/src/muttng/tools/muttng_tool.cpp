/** @ingroup muttng_bin_muttng */
/**
 * @file muttng/tools/muttng_tool.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief muttng(1) implementation
 */
#include <unistd.h>

#include <iostream>

#include "core/intl.h"

#include "ui/ui_curses.h"
#include "ui/curses_config_screen.h"

#include "muttng_tool.h"

using namespace std;

/** Usage string for @c muttng(1). */
static const char* Usage = N_("\
usage: muttng [ -nRyzZ ] [ -e <cmd> ] [ -F <file> ] [ -m <type> ] [ -f <file> ]\n\
       muttng [ -nR ] [ -e <cmd> ] [ -F <file> ] -Q <query> [ -Q <query> ] [...]\n\
       muttng [ -nR ] [ -e <cmd> ] [ -F <file> ] -A <alias> [ -A <alias> ] [...]\n\
       muttng [ -nR ] [ -e <cmd> ] [ -F <file> ] -t\n\
       muttng [ -nR ] [ -e <cmd> ] [ -F <file> ] -T\n\
       muttng [ -nx ] [ -e <cmd> ] [ -a <file> ] [ -F <file> ] [ -H <file> ] [ -i <file> ]\n\
              [ -s <subj> ] [ -b <addr> ] [ -c <addr> ] <addr> [ ... ]\n\
       muttng [ -n ] [ -e <cmd> ] [ -F <file> ] -p\n\
       muttng -v\n\
       muttng -V\n");

/** Help string for @c muttng(1). */
static const char* Options = N_("\
options:\n\
  -A <alias>\texpand the given alias\n\
  -a <file>\tattach a file to the message\n\
  -b <address>\tspecify a blind carbon-copy (BCC) address\n\
  -c <address>\tspecify a carbon-copy (CC) address\n\
  -d <level>\tlog debugging output to ~/.muttngdebug0\n\
  -e <command>\tspecify a command to be executed after initialization\n\
  -f <file>\tspecify which mailbox to read\n\
  -F <file>\tspecify an alternate muttrngc file\n\
  -g <server>\tspecify a newsserver (if compiled with NNTP)\n\
  -G\t\tselect a newsgroup (if compiled with NNTP)\n\
  -H <file>\tspecify a draft file to read header and body from\n\
  -i <file>\tspecify a file which Mutt-ng should include in the body\n\
  -d <level>\t specify debugging level of Mutt-ng\n\
  -m <type>\tspecify a default mailbox type\n\
  -n\t\tcauses Mutt-ng not to read the system Muttngrc\n\
  -p\t\trecall a postponed message\n\
  -Q <variable>\tquery a configuration variable\n\
  -R\t\topen mailbox in read-only mode\n\
  -s <subj>\tspecify a subject (must be in quotes if it has spaces)\n\
  -t\t\tprint the value of all variables to stdout\n\
  -T\t\tprint the value of all changed variables to stdout\n\
  -v\t\tshow version and compile-time definitions\n\
  -V\t\tshow warranty and license\n\
  -x\t\tsimulate the mailx send mode\n\
  -y\t\tselect a mailbox specified in your `mailboxes' list\n\
  -z\t\texit immediately if there are no messages in the mailbox\n\
  -Z\t\topen the first folder with new message, exit immediately if none\n\
  -h\t\tthis help message");

MuttngTool::MuttngTool (int argc, char** argv) : Tool (argc, argv) {
  this->ui = new UICurses (new CursesConfigScreen ());
}

MuttngTool::~MuttngTool () {}

void MuttngTool::getUsage (buffer_t* dst) {
  buffer_add_str (dst, Usage, -1);
  buffer_add_ch (dst, '\n');
  buffer_add_str (dst, Options, -1);
}

int MuttngTool::main (void) {
  int ch = 0, rc = 0;

  while ((ch = getopt (this->argc, this->argv, "" GENERIC_ARGS)) != -1) {
    switch (ch) {
      default:
        rc = genericArg (ch, optarg);
        if (rc == -1)
          displayUsage ();
        if (rc != 1)
          return (rc == 0);
        break;
    }
  }

  this->start ();
  /* do something */
  this->end ();
  return (0);
}

const char* MuttngTool::getName (void) {
  return ("muttng");
}