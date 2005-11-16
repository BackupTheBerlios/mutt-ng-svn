SUBDIRS=core libmuttng muttng

all: subdirs

clean: subdirclean
	rm -rf *.core

srcdoc:
	doxygen

test: subdirtest

depend: subdirdepend

include ./../GNUmakefile.subdirs
