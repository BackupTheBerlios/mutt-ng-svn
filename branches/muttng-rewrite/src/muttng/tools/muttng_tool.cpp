/** @ingroup muttng_bin_muttng */
/**
 * @file muttng/tools/muttng_tool.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: muttng(1)
 */
#include <unistd.h>
#include <iostream>

#include "core/intl.h"
#include "core/str.h"

#include "libmuttng/mailbox/mailbox.h"

#include "ui/ui_curses.h"
#include "muttng_tool.h"

using namespace std;

/** Usage string for @c muttng(1). */
static const char* Usage = N_("\
Usage: muttng [ -nRyzZ ] [ -e <cmd> ] [ -F <file> ] [ -m <type> ] [ -f <file> ]\n\
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
Options:\n\
  -A <alias>\texpand the given alias\n\
  -a <file>\tattach a file to the message\n\
  -b <address>\tspecify a blind carbon-copy (BCC) address\n\
  -c <address>\tspecify a carbon-copy (CC) address\n\
  -e <command>\tspecify a command to be executed after initialization\n\
  -f <file>\tspecify which mailbox to read\n\
  -g <server>\tspecify a newsserver (if compiled with NNTP)\n\
  -G\t\tselect a newsgroup (if compiled with NNTP)\n\
  -H <file>\tspecify a draft file to read header and body from\n\
  -i <file>\tspecify a file which Mutt-ng should include in the body\n\
  -d <level>\t specify debugging level of Mutt-ng\n\
  -m <type>\tspecify a default mailbox type\n\
  -p\t\trecall a postponed message\n\
  -Q <variable>\tquery a configuration variable\n\
  -R\t\topen mailbox in read-only mode\n\
  -s <subj>\tspecify a subject (must be in quotes if it has spaces)\n\
  -t\t\tprint the value of all variables to stdout\n\
  -T\t\tprint the value of all changed variables to stdout\n\
  -x\t\tsimulate the mailx send mode\n\
  -y\t\tselect a mailbox specified in your `mailboxes' list\n\
  -z\t\texit immediately if there are no messages in the mailbox\n\
  -Z\t\topen the first folder with new message, exit immediately if none\n\
  ");

MuttngTool::MuttngTool (int argc, char** argv) : Tool (argc, argv) {
  this->ui = new UICurses ();
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
  this->argc -= optind;
  this->argv += optind;

  if (!this->start ())
    return (1);
  /* do something */
  if (argc > 0)
    doIndexMenu(argv[0]);
  this->end ();
  return (0);
}

const char* MuttngTool::getName (void) {
  return ("muttng");
}

void MuttngTool::doIndexMenu(const char* url) {
  buffer_t error;
  Mailbox* folder;
  mailbox_query_status state;

  buffer_init(&error);
  if (!(folder = Mailbox::fromURL(url,&error))) {
    std::cerr << _("Error opening folder '") << (NONULL(url)) << _("': ") << (NONULL(error.str)) << std::endl;
    return;
  }

  connectSignal(folder->sigGetUsername,this,&MuttngTool::getUsername);
  connectSignal(folder->sigGetPassword,this,&MuttngTool::getPassword);

  /* XXX do loop */
  if ((state = folder->openMailbox()) == MQ_OK)
    folder->closeMailbox();
  else {
    buffer_shrink(&error,0);
    folder->strerror(state,&error);
    std::cerr << (NONULL(error.str)) << std::endl;
  }

  disconnectSignals(folder->sigGetUsername,this);
  disconnectSignals(folder->sigGetPassword,this);
  delete folder;
}

bool MuttngTool::getUsername (url_t* url) {
  char buf[64];
  if (!url)
    return false;
  std::cout << _("Enter username for ") << url->host << ": " << std::endl;
  std::cin.getline(buf,64);
  str_replace (&url->username, buf);
  return str_len(url->username)>0;
}

bool MuttngTool::getPassword (url_t* url) {
  char buf[64];
  if (!url)
    return false;
  std::cout << _("Enter password for ") << url->username << "@" << url->host << ": " << std::endl;
  std::cin.getline(buf,64);
  str_replace (&url->password, buf);
  return str_len(url->password)>0;
}
