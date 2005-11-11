/** @ingroup muttng_bin_mailx */
/**
 * @file muttng/tools/mailx_tool.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief muttng(1) implementation
 */
#include <unistd.h>

#include <iostream>

#include "core/intl.h"

#include "ui/ui_plain.h"

#include "mailx_tool.h"

using namespace std;

/** Usage string for @c muttng(1). */
static const char* Usage = N_("\
usage: muttng-mailx -v\n\
       muttng-mailx -V\n");

/** Help string for @c muttng(1). */
static const char* Options = N_("\
options:\n\
  -v\t\tshow version and compile-time definitions\n\
  -V\t\tshow warranty and license\n\
  ");

MailxTool::MailxTool (int argc, char** argv) : Tool (argc, argv) {
  this->ui = new UIPlain ();
}

MailxTool::~MailxTool () {}

void MailxTool::getUsage (buffer_t* dst) {
  buffer_add_str (dst, Usage, -1);
  buffer_add_ch (dst, '\n');
  buffer_add_str (dst, Options, -1);
}

int MailxTool::main (void) {
  int ch;

  while ((ch = getopt (this->argc, this->argv, "" GENERIC_ARGS)) != -1) {
    switch (ch) {
      default:
        if (genericArg (ch, optarg))
          return (1);
    }
  }

  this->start ();
  /* do something */
  this->end ();
  return (0);
}

const char* MailxTool::getName (void) {
  return ("muttng-mailx");
}
