## Process this file with automake to produce Makefile.in

AR=@AR@

AUTOMAKE_OPTIONS = foreign

if USE_GSS
GSSSOURCES = auth_gss.c
endif

if USE_SASL
AUTHENTICATORS = auth_sasl.c
else
AUTHENTICATORS = auth_anon.c auth_cram.c
endif

EXTRA_DIST = BUGS README TODO auth_anon.c auth_cram.c auth_gss.c auth_sasl.c

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/intl

noinst_LIBRARIES = libimap.a
noinst_HEADERS = auth.h imap_private.h message.h mx_imap.h

libimap_a_SOURCES = auth.c auth_login.c browse.c command.c imap.c imap.h mx_imap.h \
	message.c utf7.c util.c mx_imap.c $(AUTHENTICATORS) $(GSSSOURCES)
