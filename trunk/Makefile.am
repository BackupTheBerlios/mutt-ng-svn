## Process this file with automake to produce Makefile.in
## Use aclocal -I m4; automake --foreign

AUTOMAKE_OPTIONS = foreign
EXTRA_PROGRAMS = muttng_dotlock pgpringng pgpewrapng makedoc

if BUILD_IMAP
IMAP_SUBDIR = imap
IMAP_INCLUDES = -I$(top_srcdir)/imap
endif

if BUILD_POP
POP_SUBDIR = pop
POP_INCLUDES = -I$(top_srcdir)/pop
endif

if BUILD_NNTP
NNTP_SUBDIR = nntp
NNTP_INCLUDES = -I$(top_srcdir)/nntp
endif

SUBDIRS = m4 po intl doc contrib lib $(IMAP_SUBDIR) $(POP_SUBDIR) $(NNTP_SUBDIR)

bin_SCRIPTS = muttngbug fleang

BUILT_SOURCES = keymap_defs.h patchlist.c version.h

bin_PROGRAMS = muttng @DOTLOCK_TARGET@ @PGPAUX_TARGET@ @SMIMEAUX_TARGET@
muttng_SOURCES = $(BUILT_SOURCES) \
	alias.c ascii.c attach.c \
	base64.c buffer.c browser.c buffy.c \
	charset.c color.c compress.c crypt.c cryptglue.c commands.c complete.c \
	compose.c copy.c curs_lib.c curs_main.c crypt-mod.c crypt-mod.h \
	date.c \
	edit.c editmsg.c enter.c \
	flags.c filter.c from.c \
	getdomain.c \
	handler.c hash.c hcache.c hdrline.c headers.c help.c history.c hook.c \
	init.c \
	keymap.c \
	lib.c list.c \
	main.c mbox.c mbyte.c md5c.c menu.c mh.c muttlib.c mutt_idna.c mx.c \
	pager.c parse.c pattern.c postpone.c \
	query.c \
	recvattach.c recvcmd.c rfc822.c rfc1524.c rfc2047.c rfc2231.c rfc3676.c \
	score.c send.c sendlib.c sidebar.c signal.c sort.c state.c status.c system.c \
	thread.c \
	url.c utf8.c \
	wcwidth.c \
	xterm.c

muttng_LDADD = @MUTT_LIB_OBJECTS@ @LIBOBJS@ $(LIBIMAP) $(LIBPOP) $(LIBNNTP) \
	-Llib -lsane $(MUTTLIBS) $(INTLLIBS) $(LIBICONV)

muttng_DEPENDENCIES = @MUTT_LIB_OBJECTS@ @LIBOBJS@ $(top_builddir)/lib/libsane.a \
	$(LIBIMAPDEPS) $(LIBPOPDEPS) $(LIBNNTPDEPS) $(INTLDEPS)

makedoc_SOURCES = makedoc.c
makedoc_LDADD =
makedoc_DEPENDENCIES = 

# $(makedoc_OBJECTS): $(makedoc_SOURCES)
# 	$(HOST_CC) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) -c $<

# makedoc: $(makedoc_OBJECTS) $(makedoc_DEPENDENCIES)
# 	@rm -rf makedoc
# 	$(HOST_CC) $(AM_CFLAGS) $(LDFLAGS) $(makedoc_LDFLAGS) $(makedoc_OBJECTS) $(makedoc_LDADD) -o makedoc

CPP=@CPP@

DEFS=-DPKGDATADIR=\"$(pkgdatadir)\" -DSYSCONFDIR=\"$(sysconfdir)\" \
	-DBINDIR=\"$(bindir)\" -DMUTTLOCALEDIR=\"$(datadir)/locale\" \
	-DHAVE_CONFIG_H=1

INCLUDES=-I. -I$(top_srcdir) $(IMAP_INCLUDES) $(POP_INCLUDES) $(NNTP_INCLUDES) $(LIBGPGME_CFLAGS) -Iintl

CPPFLAGS=@CPPFLAGS@ -I$(includedir)

EXTRA_muttng_SOURCES = \
	account.c md5c.c mutt_sasl.c mutt_socket.c mutt_ssl.c \
	mutt_tunnel.c smime.c pgp.c pgpinvoke.c pgpkey.c \
	pgplib.c sha1.c pgpmicalg.c gnupgparse.c resize.c dotlock.c remailer.c \
	alias.h \
	buffer.h browser.h \
	enter.h \
	list.h \
	mbyte.h remailer.h url.h state.h \
	pgppacket.c mutt_idna.h mutt_libesmtp.c \
	crypt-mod-pgp-classic.c crypt-mod-smime-classic.c \
	sidebar.h state.h

EXTRA_DIST = COPYRIGHT GPL OPS OPS.PGP OPS.CRYPT OPS.SMIME TODO \
	configure acconfig.h \
	account.h alias.h attach.h recvattach.h handler.h thread.h \
	buffer.h buffy.h \
	charset.h compress.h copy.h crypthash.h \
	dotlock.h functions.h gen_defs \
	enter.h recvattach.h handler.h thread.h \
	list.h \
	globals.h hash.h history.h init.h keymap.h mutt_crypt.h \
	mapping.h md5.h mime.h mutt.h mutt_curses.h mutt_menu.h \
	mutt_sasl.h mutt_socket.h mutt_ssl.h mutt_tunnel.h \
	mbox.h mh.h mx.h pager.h pgp.h protos.h reldate.h rfc1524.h rfc2047.h \
	rfc2231.h rfc822.h rfc3676.h \
	sha1.h sort.h mime.types VERSION autogen.sh \
	_regex.h OPS.MIX remailer.c remailer.h browser.h state.h \
	mbyte.h lib.h extlib.c pgpewrap.c smime_keys.pl pgplib.h Muttngrc.head Muttngrc \
	makedoc.c stamp-doc-rc README.SSL smime.h\
	muttngbug pgppacket.h depcomp ascii.h BEWARE PATCHES patchlist.sh	\
	mkchangelog.sh cvslog2changelog.pl mutt_idna.h \
	snprintf.c regex.c mutt_libesmtp.h crypt-gpgme.h

muttng_dotlock_SOURCES = mutt_dotlock.c
muttng_dotlock_LDADD = @LIBOBJS@
muttng_dotlock_DEPENDENCIES = @LIBOBJS@

pgpringng_SOURCES = pgppubring.c pgplib.c lib.c extlib.c sha1.c md5c.c pgppacket.c ascii.c
pgpringng_LDADD = @LIBOBJS@ $(INTLLIBS) -Llib -lsane
pgpringng_DEPENDENCIES = @LIBOBJS@ $(INTLDEPS)

pgpewrapng_SOURCES = pgpewrap.c
pgpewrapng_LDADD = 
pgpewrapng_DEPENDENCIES = 

mutt_dotlock.c: dotlock.c
	cp $(srcdir)/dotlock.c mutt_dotlock.c

CLEANFILES = mutt_dotlock.c stamp-doc-rc makedoc \
	keymap_alldefs.h keymap_defs.h patchlist.c version.h

ACLOCAL_AMFLAGS = -I m4

LDADD = @LIBOBJS@ @INTLLIBS@

OPS=@OPS@

fleang:	muttbug.sh
	cp muttbug.sh fleang
	chmod +x fleang

smime_keysng: $(srcdir)/smime_keys.pl
	cp $(srcdir)/smime_keys.pl smime_keysng
	chmod +x smime_keysng

Makefile: $(BUILT_SOURCES)

keymap_defs.h: $(OPS) $(srcdir)/gen_defs
	$(srcdir)/gen_defs $(OPS) > keymap_defs.h

keymap_alldefs.h: $(srcdir)/OPS $(srcdir)/OPS.PGP $(srcdir)/OPS.MIX $(srcdir)/OPS.CRYPT $(srcdir)/OPS.SMIME $(srcdir)/gen_defs
	$(srcdir)/gen_defs $(srcdir)/OPS $(srcdir)/OPS.PGP \
		$(srcdir)/OPS.MIX $(srcdir)/OPS.CRYPT $(srcdir)/OPS.SMIME \
			> keymap_alldefs.h

version.h:
	echo "#undef MUTT_VERSION" > version.h
	if test -f "$(srcdir)/.svn/entries" ; then \
	  echo "#define MUTT_VERSION \"`cat $(srcdir)/VERSION.in`-r`grep revision $(srcdir)/.svn/entries | sort -r | uniq | head -n 1 | cut -d '"' -f 2`\"" >> version.h ; \
	else \
	  echo "#define MUTT_VERSION \"`cat $(srcdir)/VERSION.in`-`cat $(srcdir)/VERSION.date`\"" >> version.h ; \
	fi

reldate:
	echo 'const char *ReleaseDate = "'`date +%Y-%m-%d`'";' > $(srcdir)/reldate.h

patchlist.c: $(srcdir)/PATCHES $(srcdir)/patchlist.sh
	$(srcdir)/patchlist.sh < $(srcdir)/PATCHES > patchlist.c

install-exec-hook:
	if test -f $(DESTDIR)$(bindir)/mutt.dotlock && test -f $(DESTDIR)$(bindir)/muttng_dotlock ; then	\
		rm -f $(DESTDIR)$(bindir)/mutt.dotlock ;		\
		ln -sf $(DESTDIR)$(bindir)/muttng_dotlock $(DESTDIR)$(bindir)/mutt.dotlock ; \
	fi
	if test -f $(DESTDIR)$(bindir)/muttng_dotlock && test x@DOTLOCK_GROUP@ != x ; then \
		chgrp @DOTLOCK_GROUP@ $(DESTDIR)$(bindir)/muttng_dotlock && \
		chmod @DOTLOCK_PERMISSION@ $(DESTDIR)$(bindir)/muttng_dotlock || \
		{ echo "Can't fix muttng_dotlock's permissions!" >&2 ; } \
	fi

install-data-local: Muttngrc
	$(srcdir)/mkinstalldirs $(DESTDIR)$(sysconfdir)
	-if [ -f $(DESTDIR)$(pkgdatadir)/Muttngrc ] ; then \
		mv $(DESTDIR)$(pkgdatadir)/Muttngrc* $(DESTDIR)$(sysconfdir) ; \
	elif [ -f $(DESTDIR)$(pkgdatadir)/../Muttngrc ] ; then \
	 	mv $(DESTDIR)$(pkgdatadir)/../Muttngrc* $(DESTDIR)$(sysconfdir) ; \
	elif [ ! -f $(DESTDIR)$(sysconfdir)/Muttngrc ] ; then \
		$(INSTALL) -m 644 Muttngrc $(DESTDIR)$(sysconfdir) ; \
	fi
	-if [ ! -f $(DESTDIR)$(sysconfdir)/muttng-mime.types ]; then \
		$(INSTALL) -m 644 $(srcdir)/mime.types $(DESTDIR)$(sysconfdir)/muttng-mime.types; \
	fi


# Don't make this one ChangeLog - it's intended to be
# used manually.

pclean:
	cat /dev/null > $(top_srcdir)/PATCHES

check-security:
	(cd $(top_srcdir) && ./check_sec.sh)

commit-real:
	(cd $(top_srcdir) && cvs commit)

commit-changelog:
	(cd $(top_srcdir) && cvs commit -m "# changelog commit" ChangeLog)

commit: pclean check-security commit-real changelog commit-changelog

changelog:
	(cd $(top_srcdir); \
	cvs update ChangeLog; \
	sh ./mkchangelog.sh | fmt -c | cat  - ChangeLog > ChangeLog.$$$$ && mv ChangeLog.$$$$ ChangeLog; \
	$${VISUAL:-vi} ChangeLog)

ChangeLog: changelog

# kluge around automake problems.

dist-hook:
	-for file in $(BUILT_SOURCES) ; do rm -f $(distdir)/$$file ; done

mutt-dist:
	(cd $(srcdir) && ./build-release )

update-doc: stamp-doc-rc
	(cd doc && $(MAKE) update-doc)

Muttngrc: stamp-doc-rc

stamp-doc-rc: $(srcdir)/init.h makedoc Muttngrc.head
	-rm -f Muttngrc stamp-doc-rc
	$(CPP) $(INCLUDES) $(DEFS) $(CPPFLAGS) -D_MAKEDOC -C \
		$(srcdir)/init.h | ./makedoc -c | cat Muttngrc.head - > Muttngrc
	touch stamp-doc-rc


.PHONY: commit pclean check-security commit-real commit-changelog
.PHONY: changelog ChangeLog
