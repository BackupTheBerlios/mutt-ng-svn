SUBDIRS=core libmuttng muttng

all: subdirs

clean: subdirclean
	rm -rf *.core

srcdoc:
	doxygen

depend: subdirdepend

include ./../GNUmakefile.subdirs
