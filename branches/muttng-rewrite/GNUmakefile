SUBDIRS=doc src

all: subdirs

clean: subdirclean

doc:
	$(MAKE) -C ./doc doxygen manual homepage
	$(MAKE) -C ./src srcdoc

test: subdirtest

depend: subdirdepend

svn-commit:
	( $(MAKE) -C ./src/muttng/event all && \
	  $(MAKE) -C ./doc doxygen && \
	  $(MAKE) test && \
	  svn diff ) 2>&1 | less && svn commit

include GNUmakefile.subdirs

.PHONY: doc

sysconf:
	$(MAKE) -f GNUmakefile.sysconf sysconf
