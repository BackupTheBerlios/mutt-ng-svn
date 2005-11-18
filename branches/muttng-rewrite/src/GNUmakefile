SUBDIRS=core libmuttng muttng

all: subdirs

clean: subdirclean
	rm -rf *.core

srcdoc:
	doxygen

test: all subdirtest

depend: subdirdepend

.PHONY: po

po:
	$(MAKE) -C ./po all

include ./../GNUmakefile.subdirs
