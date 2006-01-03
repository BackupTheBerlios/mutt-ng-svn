/**
 * @file muttng/config/config.cpp
 * @author Copyright (C) 1996-2002 Michael R. Elkins <me@mutt.org>
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Implementation: Configuration parser
 */
#include <errno.h>
#include <cstdio>

#include "config.h"
#include "global_variables.h"

#include "libmuttng/libmuttng.h"
#include "libmuttng/util/hash.h"
#include "libmuttng/config/config_manager.h"

#include "core/io.h"
#include "core/command.h"
#include "core/intl.h"
#include "core/str.h"
#include "core/conv.h"

static Hash<buffer_t*>* FileStack = NULL;

bool Config::config_read_file(buffer_t* path, buffer_t* error) {
  FILE* fp = NULL;
  pid_t filter;
  buffer_t line, localline, expand;
  size_t count;
  char* p;
  /* hash code for path so we don't need to compute several times */
  unsigned int code = hash_key(path->str);

  DEBUGPRINT(D_PARSE,("reading config: '%s'",path->str));

  if (FileStack->exists(path->str,code)) {
    buffer_add_str(error,_("Loop in config reading detected: '"),-1);
    buffer_add_buffer(error,path);
    buffer_add_str(error,_("' is being parsed already"),-1);
    DEBUGPRINT(D_PARSE,("%s",error->str));
    return false;
  }

  buffer_init(&line);
  buffer_init(&localline);
  buffer_init(&expand);

  if (!(fp = io_open_read(path->str,&filter))) {
    buffer_add_str(error,_("Couldn't read config '"),-1);
    buffer_add_buffer(error,path);
    buffer_add_str(error,_("': "),-1);
    buffer_add_str(error,strerror(errno),-1);
    return false;
  }

  FileStack->add(path->str,path,code);

  while ((count = io_readline(&line,fp)) != 0) {
    /* use this line for reading */
    buffer_t* use = NULL;

    if (ConfigCharset && *ConfigCharset) {
      /* if $config_charset is set, convert line and mangle use ptr */
      buffer_shrink(&localline,0);
      buffer_add_buffer(&localline,&line);
      conv_iconv(&localline,ConfigCharset,Charset);
      use = &localline;
    } else
      /* with no $config_charset, use as-is */
      use = &line;

    buffer_shrink(&expand,0);
    if (!buffer_expand(&expand,use,ConfigManager::get)) continue;

    p = expand.str;
    SKIPWS(p);
    /* skip empty and comment lines */
    if (!p || !*p || *p == '#') continue;

    DEBUGPRINT(D_PARSE,("got config line: '%s'",p));
  }

  io_fclose(&fp);
  command_filter_wait(filter);

  buffer_free(&expand);
  buffer_free(&line);
  buffer_free(&localline);

  FileStack->del(path->str,code);

  return true;
}

bool Config::read(buffer_t* error) {
  buffer_t path;

  FileStack = new Hash<buffer_t*>(20);

  buffer_init(&path);
  buffer_add_str(&path,Homedir,-1);
  buffer_add_str(&path,"/.muttngrc",10);
  bool rc = config_read_file(&path,error);
  buffer_free(&path);

  delete FileStack;
  return rc;
}

void Config::reg() {
  ConfigManager::regOption(new StringOption("config_charset",NULL,&ConfigCharset));
}
