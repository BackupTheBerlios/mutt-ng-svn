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

test: all subdirtest

depend: subdirdepend

.PHONY: po

po:
	$(MAKE) -C ./po all

include ./../GNUmakefile.subdirs
include ./../GNUmakefile.config.mine
include ./../GNUmakefile.config
