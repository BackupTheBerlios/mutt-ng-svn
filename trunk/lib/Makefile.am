## Process this file with automake to produce Makefile.in

AR=@AR@

AUTOMAKE_OPTIONS = foreign

INCLUDES = -I$(top_srcdir) -I../intl

noinst_LIBRARIES = libsane.a
noinst_HEADERS = mem.h str.h exit.h intl.h list.h rx.h

libsane_a_SOURCES = mem.c exit.c str.c list.c rx.h \
		    mem.h exit.h str.h list.h rx.c intl.h
