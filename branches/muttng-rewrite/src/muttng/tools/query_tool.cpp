/** @ingroup muttng_bin_query */
/**
 * @file muttng/tools/muttng_query.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: muttng-query(1)
 */
#include <unistd.h>
#include <iostream>

#include "core/intl.h"
#include "core/str.h"

#include "libmuttng/mailbox/mailbox.h"

#include "ui/ui_plain.h"
#include "query_tool.h"

using namespace std;

/** Usage string for @c muttng(1). */
static const char* Usage = N_("\
Usage: muttng -v\n\
       muttng -V\n");

/** Help string for @c muttng(1). */
static const char* Options = N_("\
Options:\n\
  ");

QueryTool::QueryTool (int argc, char** argv) : Tool (argc, argv) {
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
    doLogin(argv[0]);
  this->end ();
  return (0);
}

const char* QueryTool::getName (void) {
  return ("muttng-query");
}

void QueryTool::doLogin(const char* url) {
  buffer_t error;
  Mailbox* folder;
  mailbox_query_status state;

  buffer_init(&error);
  if (!(folder = Mailbox::fromURL(url,&error))) {
    std::cerr << _("Error opening folder '") << (NONULL(url)) << _("': ") << (NONULL(error.str)) << std::endl;
    return;
  }

  connectSignal(folder->sigGetUsername,this,&QueryTool::getUsername);
  connectSignal(folder->sigGetPassword,this,&QueryTool::getPassword);

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
    str_replace(&url->username,value.str);
  }
  buffer_free(&value);
  buffer_free(&prompt);
  return rc && str_len(url->password)>0;
}
