## Process this file with automake to produce Makefile.in

AR=@AR@

AUTOMAKE_OPTIONS = foreign

INCLUDES = -I$(top_srcdir) -I../intl

noinst_LIBRARIES = libnntp.a
noinst_HEADERS = nntp.h mx_nntp.h

libnntp_a_SOURCES = nntp.h mx_nntp.h \
		    nntp.c mx_nntp.c newsrc.c
