/** @ingroup muttng_bin_sync */
/**
 * @file muttng/tools/sync_tool.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: muttng-sync(1)
 *
 * This file is published under the GNU General Public License.
 */
#include <unistd.h>

#include <iostream>

#include "core/intl.h"

#include "ui/ui_plain.h"

#include "sync_tool.h"

using namespace std;

/** Usage string for @c muttng(1). */
static const char* Usage = N_(" ");

/** Help string for @c muttng(1). */
static const char* Options = N_(" ");

SyncTool::SyncTool (int argc, char** argv) : Tool (argc, argv) {
  this->ui = new UIPlain(getName());
}

SyncTool::~SyncTool () {}

void SyncTool::getUsage (buffer_t* dst) {
  buffer_add_str (dst, Usage, -1);
  buffer_add_ch (dst, '\n');
  buffer_add_str (dst, Options, -1);
}

int SyncTool::main (void) {
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

  if (!this->start ())
    return (1);
  /* do something */
  this->end ();
  return (0);
}

const char* SyncTool::getName (void) {
  return ("muttng-sync");
}
