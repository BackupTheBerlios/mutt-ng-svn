SUBDIRS=doc src

all: sysconf subdirs

clean: subdirclean

doc:
	$(MAKE) -C ./doc doxygen manual homepage
	$(MAKE) -C ./src srcdoc

test: subdirtest

depend: subdirdepend

svn-commit:
	( $(MAKE) -C ./src/muttng/event all && \
	  $(MAKE) -C ./src/muttng/config all && \
	  $(MAKE) -C ./doc doxygen && \
	  svn diff ) 2>&1 | less && svn commit

include GNUmakefile.subdirs

.PHONY: doc

sysconf:
	$(MAKE) -f GNUmakefile.sysconf sysconf
