/** @ingroup muttng */
/**
 * @file muttng/tools/main.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Common wrapper for <code>main()</code>
 */
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "core/str.h"

#include "muttng_tool.h"
#include "conf_tool.h"
#include "mailx_tool.h"
#include "sync_tool.h"

/**
 * Common @c main() function for all tools.
 * Depending on the name it's invoked with, it creates the proper user
 * interface and the tool with it.
 * @param argc Command-line argument count.
 * @param argv Command-line argument vector.
 * @return Tool::main().
 */
int main (int argc, char** argv) {
  Tool* tool;
  char* p = NULL;

  /* get our binary name */
  if (!argv || !(p = strrchr (argv[0], '/')) || !*(p++))
    p = argv[0];

  if (!argv[0])
    tool = new MuttngTool (argc, argv);
  else if (str_eq (p, "muttng"))
    tool = new MuttngTool (argc, argv);
  else if (str_eq (p, "muttng-conf"))
    tool = new ConfTool (argc, argv);
  else if (str_eq (p, "muttng-mailx"))
    tool = new MailxTool (argc, argv);
  else if (str_eq (p, "muttng-sync"))
    tool = new SyncTool (argc, argv);
  else
    tool = new MuttngTool (argc, argv);
  return (tool->main ());
}
