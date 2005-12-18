/** @ingroup muttng_bin_query */
/**
 * @file muttng/tools/query_tool.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: muttng-query(1)
 */
#include <unistd.h>
#include <iostream>

#include "core/intl.h"
#include "core/str.h"
#include "core/mem.h"

#include "libmuttng/mailbox/mailbox.h"

#include "ui/ui_plain.h"
#include "query_tool.h"

using namespace std;

/** Usage string for @c muttng(1). */
static const char* Usage = N_("\
Usage: muttng-query [-c] URL...\n\
  ");

/** Help string for @c muttng(1). */
static const char* Options = N_("\
Options:\n\
  -c\tPrint output in colon-separated fields.\n\
  ");

QueryTool::QueryTool (int argc, char** argv) : Tool (argc, argv),colon(false) {
  this->ui = new UIPlain ();
}

QueryTool::~QueryTool () {}

void QueryTool::getUsage (buffer_t* dst) {
  buffer_add_str (dst, Usage, -1);
  buffer_add_ch (dst, '\n');
  buffer_add_str (dst, Options, -1);
}

int QueryTool::main (void) {
  int ch = 0, rc = 0;

  while ((ch = getopt (this->argc, this->argv, "c" GENERIC_ARGS)) != -1) {
    switch (ch) {
    case 'c': colon = true; break;
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

  if (argc==0) {
    displayUsage();
    return 1;
  }

  if (!this->start ())
    return (1);
  /* do something */
  doURLs();
  this->end ();
  return (0);
}

const char* QueryTool::getName (void) {
  return ("muttng-query");
}

void QueryTool::folderStats(Mailbox* folder) {
  if (!folder) return;
  static const char* n_sep[] = { N_("Folder: "), N_(", total/new/old/flagged: "), N_("/"), "" };
  static const char* c_sep[] = { "",             ":",                             ":",     ":" };
  const char** sep = colon?c_sep:n_sep;
  buffer_t tmp;
  buffer_init(&tmp);
  folder->getURL(&tmp);
  std::cout<<sep[0]<<tmp.str<<sep[1]<<folder->msgTotal()<<sep[2]<<folder->msgNew()<<sep[2]<<
    folder->msgOld()<<sep[2]<<folder->msgFlagged()<<sep[3]<<std::endl;
  buffer_free(&tmp);
}

void QueryTool::doURLs() {
  if (argc==0) return;

  buffer_t error;
  Mailbox* folder;
  mailbox_query_status state;
  buffer_init(&error);

  for (int i = 0; i < argc; i++) {

    buffer_shrink(&error,0);

    if (!(folder = Mailbox::fromURL(argv[i],&error))) {
      std::cerr << _("Error opening folder '") << (NONULL(argv[i])) << _("': ") << (NONULL(error.str)) << std::endl;
      continue;
    }

    connectSignal(folder->sigGetUsername,this,&QueryTool::getUsername);
    connectSignal(folder->sigGetPassword,this,&QueryTool::getPassword);

    if ((state = folder->openMailbox()) == MQ_OK) {
      if ((state = folder->checkMailbox())) {
        buffer_shrink(&error,0);
        folder->strerror(state,&error);
        std::cerr << _("Error opening folder '") << (NONULL(argv[i])) << _("': ") << (NONULL(error.str)) << std::endl;
      } else
        folderStats(folder);
      folder->closeMailbox();
    } else {
      buffer_shrink(&error,0);
      folder->strerror(state,&error);
      std::cerr << (NONULL(error.str)) << std::endl;
    }

    disconnectSignals(folder->sigGetUsername,this);
    disconnectSignals(folder->sigGetPassword,this);

    delete folder;
  }
  buffer_free(&error);
}

bool QueryTool::getUsername (url_t* url) {
  buffer_t prompt, value;
  bool rc = false;
  buffer_init(&prompt);
  buffer_init(&value);
  buffer_add_str(&prompt,_("Enter username for "),-1);
  buffer_add_str(&prompt,url->host,-1);
  if ((rc = ui->enterValue(&value,&prompt,64))) {
    rc = true;
    str_replace(&url->username,value.str);
  }
  buffer_free(&value);
  buffer_free(&prompt);
  return rc && str_len(url->username)>0;
}

bool QueryTool::getPassword (url_t* url) {
  buffer_t prompt, value;
  bool rc = false;
  buffer_init(&prompt);
  buffer_init(&value);
  buffer_add_str(&prompt,_("Enter password for "),-1);
  buffer_add_str(&prompt,url->username,-1);
  buffer_add_ch(&prompt,'@');
  buffer_add_str(&prompt,url->host,-1);
  if ((rc = ui->enterValue(&value,&prompt,64))) {
    rc = true;
    str_replace(&url->password,value.str);
  }
  buffer_free(&value);
  buffer_free(&prompt);
  return rc && str_len(url->password)>0;
}
