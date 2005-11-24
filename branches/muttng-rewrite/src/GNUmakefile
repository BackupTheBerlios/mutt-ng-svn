SUBDIRS=core libmuttng muttng

all: subdirs

clean: subdirclean
	rm -rf *.core

Doxyfile: Doxyfile.in GNUmakefile
	sed \
	  -e "s#__UNITPPDIR__#$(UNITPPDIR)#g" \
	  -e "s#__CURDIR__#$(CURDIR)#g" \
	  < $< > $@

srcdoc: Doxyfile
	doxygen
	@ for i in $(shell find ./../doxygen -name '*.html') ; do \
	  echo "cleanup $$i" ; \
	  tidy -q -i -m -utf8 $$i >/dev/null 2>&1 || true ; \
	done
	find ./../doxygen -name '*.html' | ./../contrib/html_head_number.pl

test: all subdirtest

depend: subdirdepend

.PHONY: po

po:
	$(MAKE) -C ./po all

include ./../GNUmakefile.subdirs
include ./../GNUmakefile.config.mine
include ./../GNUmakefile.config
