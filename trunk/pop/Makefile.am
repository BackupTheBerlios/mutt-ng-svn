## Process this file with automake to produce Makefile.in

AR=@AR@

AUTOMAKE_OPTIONS = foreign

INCLUDES = -I$(top_srcdir) -I../intl

noinst_LIBRARIES = libpop.a
noinst_HEADERS = pop.h mx_pop.h

libpop_a_SOURCES = pop.c pop_auth.c pop_lib.c pop.h mx_pop.h mx_pop.c
