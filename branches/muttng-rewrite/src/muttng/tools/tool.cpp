/** @ingroup muttng_bin */
/**
 * @file muttng/tools/tool.cpp
 * @author Rocco Rutte <pdmef@cs.tu-berlin.de>
 * @brief Common tool superclass implementation
 */
#include <iostream>

#include <sys/utsname.h>

#include "core/version.h"
#include "core/intl.h"
#include "core/buffer.h"

#include "libmuttng/version.h"
#include "libmuttng/debug.h"

#include "ui/ui_plain.h"

#include "config/global_variables.h"

#include "tool.h"

using namespace std;

/** How to reach us */
static const char *ReachingUs = N_("\
To contact the developers, please mail to <mutt-ng-devel@lists.berlios.de>.\n\
To visit the Mutt-ng homepage go to http://www.muttng.org.\n\
To report a bug, please use the fleang(1) utility.\n");

/** Warranty notice */
static const char *Notice = N_("\
Muttng comes with ABSOLUTELY NO WARRANTY; for details type `muttng -V'.\n\
Muttng is free software, and you are welcome to redistribute it\n\
under certain conditions; type `muttng -V' for details.\n");

/** Copyright for original mutt */
static const char *Copyright_Mutt = N_("\
Muttng is partially based on Mutt:\n\
  Copyright (C) 1996-2004 Michael R. Elkins <me@mutt.org>\n\
  Copyright (C) 1996-2002 Brandon Long <blong@fiction.net>\n\
  Copyright (C) 1997-2005 Thomas Roessler <roessler@does-not-exist.org>\n\
  Copyright (C) 1998-2005 Werner Koch <wk@isil.d.shuttle.de>\n\
  Copyright (C) 1999-2005 Brendan Cully <brendan@kublai.com>\n\
  Copyright (C) 1999-2002 Tommi Komulainen <Tommi.Komulainen@iki.fi>\n\
  Copyright (C) 2000-2002 Edmund Grimley Evans <edmundo@rano.org>\n");

/** Copyright from us */
static const char *Copyright_Muttng = N_("\
Muttng is Copyright (C) 2005:\n\
  Copyright (C) 2005 Andreas Krennmair <ak@synflood.at>\n\
  Copyright (C) 2005 Nico Golde <nico@ngolde.de>\n\
  Copyright (C) 2005 Rocco Rutte <pdmef@cs.tu-berlin.de>\n\
\n\
  Lots of others not mentioned here contributed lots of code,\n\
  fixes, and suggestions.\n");

/** short license pointer */
static const char* GPL = N_("\
This program is free software; you can redistribute it and/or modify\n\
it under the terms of the GNU General Public License as published by\n\
the Free Software Foundation; either version 2 of the License, or\n\
(at your option) any later version.\n\
\n\
This program is distributed in the hope that it will be useful,\n\
but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
GNU General Public License for more details.\n\
\n\
You should have received a copy of the GNU General Public License\n\
along with this program; if not, write to the Free Software\n\
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.\n\
");

static const char* GenericOptions = N_("\
Generic options:\n\
  -d <level>\tDebug at <level>\n\
  -F <file>\tRead <file> instead of default user configuration\n\
  -h\t\tThis help screen\n\
  -n\t\tDo not read the system configuration file\n\
  -v\t\tShow version info and compile-time options\n\
  -V\t\tShow warranty and license");

Tool::Tool (int argc, char** argv) {
  this->argc = argc;
  this->argv = argv;
  this->readGlobal = true;
  this->altConfig = NULL;
  this->startDebug = 0;
  this->libmuttng = NULL;
}

Tool::~Tool () {
  if (this->libmuttng)
    delete (this->libmuttng);
  delete (this->ui);
}

int Tool::genericArg (unsigned char c, const char* arg) {
  int rc = 1;
  switch (c) {
    case 'v': displayVersion (); rc = 0; break;
    case 'V': displayWarranty (); rc = 0; break;
    case 'h': displayUsage (); rc = 0; break;
    case 'n': readGlobal = false; break;
    case 'F': altConfig = arg; break;
    case 'd': startDebug = atoi (optarg); break;
    case '?':
    default:
      return (-1);
  }
  return (rc);
}

bool Tool::start (void) {
  buffer_t error;
  UIPlain ui;

  buffer_init ((&error));
  config = new Config ();
  if (!config->init (&ui))
    return (false);
  muttngInit (Homedir, getName (), Umask);
  this->libmuttng = new LibMuttng (Homedir, Umask);
  if (startDebug) {
    setDebugLevel (startDebug);
    this->libmuttng->setDebugLevel (startDebug);
  }
  if (!config->read (this->readGlobal, this->altConfig, &error))
    return (false);
  return (this->ui->start ());
}

bool Tool::end (void) {
  return (this->ui->end ());
}

void Tool::doName (buffer_t* dst) {
  buffer_add_str (dst, getName (), -1);
  buffer_add_str (dst, ", version devel/rXYZ (", -1);
  buffer_add_str (dst, CORE_VERSION, -1);
  buffer_add_str (dst, ", ", 2);
  buffer_add_str (dst, LIBMUTTNG_VERSION, -1);
  buffer_add_ch (dst, ')');
}

void Tool::doCopyright (buffer_t* dst) {
  buffer_add_str (dst, Copyright_Mutt, -1);
  buffer_add_ch (dst, '\n');
  buffer_add_str (dst, Copyright_Muttng, -1);
}

void Tool::doContact (buffer_t* dst) {
  buffer_add_str (dst, ReachingUs, -1);
}

void Tool::doSystem (buffer_t* dst) {
  struct utsname uts;

  uname (&uts);
  buffer_add_str (dst, "System:\n  ", -1);
#ifdef _AIX
  buffer_add_str (dst, uts.sysname, -1);
  buffer_add_ch (dst, ' ');
  buffer_add_str (dst, uts.version, -1);
  buffer_add_ch (dst, ' ');
  buffer_add_str (dst, uts.release, -1);
#elif defined (SCO)
  buffer_add_str (dst, "SCO ");
  buffer_add_str (dst, uts.release, -1);
#else
  buffer_add_str (dst, uts.sysname, -1);
  buffer_add_ch (dst, ' ');
  buffer_add_str (dst, uts.release, -1);
#endif

  buffer_add_str (dst, "\nInternal Libraries:\n  ", -1);
  buffer_add_str (dst, CORE_VERSION, -1);
  buffer_add_str (dst, "\n  ", 3);
  buffer_add_str (dst, LIBMUTTNG_VERSION, -1);

  buffer_add_str (dst, "\nExternal Libraries:\n  ", -1);
}

void Tool::doLicense (buffer_t* dst) {
  buffer_add_str (dst, GPL, -1);
}

void Tool::displayVersion (void) {
  buffer_t name, copyright, options, reach;

  buffer_init ((&name));
  buffer_init ((&copyright));
  buffer_init ((&options));
  buffer_init ((&reach));

  doName (&name);
  doCopyright (&copyright);
  buffer_add_ch (&copyright, '\n');
  buffer_add_str (&copyright, Notice, -1);
  doSystem (&options);
  doContact (&reach);

  ui->displayVersion (name.str, copyright.str, options.str, reach.str);

  buffer_free (&name);
  buffer_free (&copyright);
  buffer_free (&options);
  buffer_free (&reach);
}

void Tool::displayWarranty (void) {
  buffer_t name, copyright, reach, gpl;

  buffer_init ((&name));
  buffer_init ((&copyright));
  buffer_init ((&reach));
  buffer_init ((&gpl));

  doName (&name);
  doCopyright (&copyright);
  doLicense (&gpl);
  doContact (&reach);

  ui->displayWarranty (name.str, copyright.str, gpl.str, reach.str);

  buffer_free (&name);
  buffer_free (&copyright);
  buffer_free (&gpl);
  buffer_free (&reach);
}

void Tool::displayUsage (void) {
  buffer_t usage;

  buffer_init ((&usage));
  doName (&usage);
  buffer_add_ch (&usage, '\n');
  getUsage (&usage);
  buffer_add_ch (&usage, '\n');
  buffer_add_str (&usage, GenericOptions, -1);
  ui->displayMessage (usage.str);
  buffer_free (&usage);
}
