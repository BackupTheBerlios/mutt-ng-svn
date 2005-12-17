include ./../GNUmakefile.config.mine
include ./../GNUmakefile.whereis
include ./../GNUmakefile.config

ifeq ($(call whereis,dot,),)
HAVEDOT=NO
else
HAVEDOT=YES
endif

SUBDIRS=core libmuttng muttng

all: subdirs

clean: subdirclean
	rm -rf *.core

Doxyfile: Doxyfile.in GNUmakefile ../GNUmakefile.config.mine
	sed \
	  -e "s#__UNITPPDIR__#$(UNITPPDIR)#g" \
	  -e "s#__MUTTNGDIR__#$(shell echo $(CURDIR) | sed 's#[^/]*/[^/]*$$##')#g" \
	  -e "s#__CCDOCS__#$(CCDOCS)#g" \
	  -e "s#__HAVE_DOT__#$(HAVEDOT)#g" \
	  < $< > $@

srcdoc: Doxyfile
ifneq ($(DOXYGEN),)
	$(DOXYGEN)
ifneq ($(TIDY),)
	@ echo "Running tidy..."
	@ find ./../doxygen -name '*.html' | xargs -I @ $(TIDY) -q -i -m -utf8 @ >/dev/null 2>&1 || true
endif
endif
	@ echo "Numbering headlines"
	@ find ./../doxygen -name '*.html' | xargs ./../contrib/html_head_number.pl

test: all subdirtest

depend: subdirdepend

.PHONY: po

po:
	$(MAKE) -C ./po all

include ./../GNUmakefile.subdirs
