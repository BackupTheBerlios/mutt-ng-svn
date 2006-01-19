/** @ingroup muttng_bin_query */
/**
 * @file muttng/tools/query_tool.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: muttng-query(1)
 *
 * This file is published under the GNU General Public License.
 */
#include <unistd.h>
#include <iostream>

#include "core/intl.h"
#include "core/str.h"
#include "core/mem.h"
#include "core/net.h"
#include "core/conv.h"

#include "libmuttng/mailbox/mailbox.h"

#include "ui/ui_plain.h"
#include "query_tool.h"

using namespace std;

/** Usage string for @c muttng(1). */
static const char* Usage = N_("\
Usage: muttng-query [-q] [-m] URL...\n\
       muttng-query -i Host...\n\
       muttng-query -c/-C\n\
  ");

/** Help string for @c muttng(1). */
static const char* Options = N_("\
Options:\n\
  -c\t\tList supported MIME character sets\n\
  -C\t\tList supported character set aliases\n\
  -i\t\tConvert given IDN domain names using punycode.\n\
  -m\t\tPrint output in machine-readable format.\n\
  ");

QueryTool::QueryTool (int argc, char** argv) : Tool (argc, argv),colon(false) {
  this->ui = new UIPlain(getName());
}

QueryTool::~QueryTool () {}

void QueryTool::getUsage (buffer_t* dst) {
  buffer_add_str (dst, Usage, -1);
  buffer_add_ch (dst, '\n');
  buffer_add_str (dst, Options, -1);
}

int QueryTool::main (void) {
  int ch = 0, rc = 0, args = 1;
  QueryTool::modes mode = M_MAILBOX_QUERY;

  while ((ch = getopt (this->argc, this->argv, "cCim" GENERIC_ARGS)) != -1) {
    switch (ch) {
    case 'm': colon = true;
      /* fallthrough */
    case 'q': Tool::quiet = true; break;
    case 'i': mode = M_IDN; break;
    case 'c': mode = M_CHARSET_MIME; args = 0; break;
    case 'C': mode = M_CHARSET_ALIAS; args = 0; break;
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

  if (argc<args) {
    displayUsage();
    return 1;
  }

  if (!this->start ())
    return (1);

  /* do something */
  switch(mode) {
  case M_MAILBOX_QUERY: doURLs(); break;
  case M_IDN: doIDN(); break;
  case M_CHARSET_ALIAS:
  case M_CHARSET_MIME:
    doCharsets(mode==M_CHARSET_MIME);
    break;
  }
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

  Mailbox* folder;

  for (int i = 0; i < argc; i++) {
    if (!(folder = Mailbox::fromURL(argv[i])))
      continue;
    connectSignal(folder->sigGetUsername,this,&QueryTool::getUsername);
    connectSignal(folder->sigGetPassword,this,&QueryTool::getPassword);
    if (folder->openMailbox() == MQ_OK) {
      if (folder->checkMailbox() == MQ_OK)
        folderStats(folder);
      folder->closeMailbox();
    }
    /* not really needed as folder gets destroyed, but... */
    disconnectSignals(folder->sigGetUsername,this);
    disconnectSignals(folder->sigGetPassword,this);
    delete folder;
  }
}

void QueryTool::doIDN() {
  buffer_t in,out;
  buffer_init(&in);
  buffer_init(&out);
  for (int i = 0; i < argc; i++) {
    buffer_shrink(&in,0);
    buffer_shrink(&out,0);
    buffer_add_str(&in,argv[i],-1);
    net_local2idn(&out,&in,Charset);
    std::cout<<in.str<<":"<<(colon?"":" ")<<out.str<<(colon?":":"")<<std::endl;
  }
  buffer_free(&in);
  buffer_free(&out);
}

/**
 * Callback for conv_charset_list(): just print line
 * @param line Line.
 * @return 1.
 */
static int printline1(const char* line) { std::cout<<(NONULL(line))<<std::endl; return 1; }

/**
 * Callback for conv_charset_list(): just print line for colon mode
 * @param line Line.
 * @return 1.
 */
static int printline2(const char* line) { std::cout<<(NONULL(line))<<":"<<std::endl; return 1; }

void QueryTool::doCharsets(bool mime) {
  conv_charset_list(mime,colon?printline2:printline1);
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
