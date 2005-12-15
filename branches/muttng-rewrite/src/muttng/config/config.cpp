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
#include "libmuttng/config/config_manager.h"

#include "core/io.h"
#include "core/command.h"
#include "core/intl.h"

bool Config::config_read_file(buffer_t* path, buffer_t* error) {
  FILE* fp = NULL;
  pid_t filter;
  buffer_t line;
  size_t count;

  buffer_init(&line);

  DEBUGPRINT(D_PARSE,("reading config: '%s'",path->str));
  if (!(fp = io_open_read(path->str,&filter))) {
    buffer_add_str(error,_("Couldn't read config '"),-1);
    buffer_add_buffer(error,path);
    buffer_add_str(error,_("': "),-1);
    buffer_add_str(error,strerror(errno),-1);
    return false;
  }

  while ((count = io_readline(&line,fp)) != 0) {
    /* XXX $config_charset */
    DEBUGPRINT(D_PARSE,("got config line: '%s'",line.str));
  }

  io_fclose(&fp);
  if (filter != -1)
    command_filter_wait(filter);
  buffer_free(&line);
  return true;
}

bool Config::read(buffer_t* error) {
  buffer_t path;

  buffer_init(&path);
  buffer_add_str(&path,Homedir,-1);
  buffer_add_str(&path,"/.muttngrc",10);
  bool rc = config_read_file(&path,error);
  buffer_free(&path);
  return rc;
}

void Config::reg() {
  ConfigManager::reg(new StringOption("config_charset",NULL,&ConfigCharset));
}
