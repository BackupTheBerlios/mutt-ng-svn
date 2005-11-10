SUBDIRS=mailbox

include $(CURDIR)/../../GNUmakefile.config.default

all: subdirs libmuttng.a

clean: subdirclean
	rm -rf libmuttng.a

libmuttng.a:
	$(AR) -cru $@ $^ $(wildcard ./mailbox/*.o)
	$(RANLIB) $@

depend: subdirdepend

include ./../../GNUmakefile.subdirs
